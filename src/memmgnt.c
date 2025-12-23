#include "memmgnt.h"

#include "assert.h"
#include "interrupts.h"

#include <stddef.h>
#include <stdint.h>

extern size_t __kernel_code_end;

void memmove(void* dest, const void* src, size_t count) {
  ptrdiff_t distance = (uint8_t*) dest - (const uint8_t*) src;
  if (distance > 0 && (size_t) distance < count) {
    for (size_t i = 0; i < count; i++) {
      ((uint8_t*) dest)[count - i - 1] = ((const uint8_t*) src)[count - i - 1];
    }
    return;
  }
  for (size_t i = 0; i < count; i++) {
    ((uint8_t*) dest)[i] = ((const uint8_t*) src)[i];
  }
}

// void memcpy(void* dest, const void* src, size_t count);
// void (*memcpy)(void*, const void*, size_t) = memmove;

void memzero(void* dest, size_t count) {
  memset(dest, 0, count);
}

void memset(void* dest, int ch, size_t count) {
  for (size_t i = 0; i < count; i++) {
    ((uint8_t*) dest)[i] = (uint8_t) ch;
  }
}

size_t strlen(const char* str) {
  size_t r = 0;
  while (*str) {
    r++;
    str++;
  }
  return r;
}

#define ARENA_COUNT 2

static void* const ARENA_START[ARENA_COUNT] = {NULL, (void*) 0x100000};
static void* const ARENA_END[ARENA_COUNT]   = {(void*) 0x80000, (void*) 0x400000};

static void* current[2] = {ARENA_START[0], ARENA_START[1]};

void init_immortal_allocator(void) {
  current[0] = (void*) &__kernel_code_end;
}

void* malloc_immortal(size_t size, size_t alignment) {
  if (!size) {
    return NULL;
  }
  if (alignment == 0) {
    alignment = 1;
  }
  pushfd();
  cli();
  for (size_t i = 1; i < ARENA_COUNT; i++) {
    void* cur = (void*) (((size_t) current[i] + (alignment - 1)) / alignment * alignment);
    if ((cur < ARENA_END[i]) && (size_t) ((uint8_t*) ARENA_END[i] - (uint8_t*) cur) >= size) {
      current[i] = (void*) ((uint8_t*) cur + size);
      popfd();
      return cur;
    }
  }
  kernel_panic("Not enough memory in immortal allocator");
  __builtin_unreachable();
}

void* calloc_immortal(size_t size, size_t alignment) {
  void* result = malloc_immortal(size, alignment);
  memzero(result, size);
  return result;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
  if (!num)
    return 0;
  const int8_t* p1 = ptr1;
  const int8_t* p2 = ptr2;
  while (num-- && *p1++ == *p2++) {
  }
  return (int) *--p1 - (int) *--p2;
}

#define PAGE_SPACE_START 0x400000

//TODO: make it actually SCAN memory while in BIOS
#ifndef PAGE_SPACE_END
  #define PAGE_SPACE_END 0x40000000
#endif

#define PAGE_SIZE 0x1000

static void* frame_edge      = (void*) PAGE_SPACE_START;
static void* frame_free_list = NULL;

//todo high-half kernel & IN-VA context switching
//todo swapping
void* malloc_page(void) {
  void* page;
  if (frame_free_list) {
    page            = frame_free_list;
    frame_free_list = *(void**) frame_free_list;
  } else {
    if ((size_t) frame_edge >= (size_t) (PAGE_SPACE_END - PAGE_SIZE))
      kernel_panic("Out of memory. Download more ram at downloadmoreram.com\n");
    page       = frame_edge;
    frame_edge = (uint8_t*) frame_edge + PAGE_SIZE;
  }
  return page;
}

void* calloc_page(void) {
  void* page = malloc_page();
  memzero(page, PAGE_SIZE);
  return page;
}

void free_page(void* page) {
  *(void**) page  = frame_free_list;
  frame_free_list = page;
}

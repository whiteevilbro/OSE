#include "memmgnt.h"

#include "assert.h"
#include "interrupts.h"

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
void (*memcpy)(void*, const void*, size_t) = memmove;

void memzero(void* dest, size_t count) {
  memset(dest, 0, count);
}

void memset(void* dest, int ch, size_t count) {
  for (size_t i = 0; i < count; i++) {
    ((uint8_t*) dest)[i] = (uint8_t) ch;
  }
}

#define ARENA_COUNT 2

extern void halt();
static void* const ARENA_START[ARENA_COUNT] = {NULL, (void*) 0x100000};
static void* const ARENA_END[ARENA_COUNT] = {(void*) 0x80000, (void*) 0x400000};

static void* current[2] = {ARENA_START[0], ARENA_START[1]};

void init_TITLE_CARD_allocator() {
  current[0] = (void*) &__kernel_code_end;
}

void* malloc_TITLE_CARD(size_t size, size_t alignment) {
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
  kernel_panic("not enough memory in TITLE_CARD allocator");
  __builtin_unreachable();
}

void* calloc_TITLE_CARD(size_t size, size_t alignment) {
  void* result = malloc_TITLE_CARD(size, alignment);
  memzero(result, size);
  return result;
}

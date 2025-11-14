#ifndef MEMMGNT_H_
#define MEMMGNT_H_

#include <stddef.h>

#define Alignas(a) __attribute__((aligned(a)))

void memmove(void* dest, const void* src, size_t count);
// void memcpy(void* dest, const void* src, size_t count);
void memzero(void* dest, size_t count);
void memset(void* dest, int ch, size_t count);
void init_immortal_allocator(void);
void* malloc_immortal(size_t size, size_t alignment);
void* calloc_immortal(size_t size, size_t alignment);
size_t strlen(const char* str);

#endif

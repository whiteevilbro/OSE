#ifndef MEMMGNT_H_
#define MEMMGNT_H_

#include <stddef.h>
#include <stdint.h>

void memmove(void* dest, const void* src, size_t count);
// void memcpy(void* dest, const void* src, size_t count);
void memzero(void* dest, size_t count);
void memset(void* dest, int ch, size_t count);
void init_TITLE_CARD_allocator();
void* malloc_TITLE_CARD(size_t size, size_t alignment);
void* calloc_TITLE_CARD(size_t size, size_t alignment);

#endif
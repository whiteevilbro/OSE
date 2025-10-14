#include "assert.h"
#include "memmgnt.h"
#include "vga.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void kernel_entry(uint8_t columns, uint8_t row, uint8_t column) {
  size_t al[3] = {4, 8};
  size_t sz[5] = {4, 16, 32, 1024, 1024 * 10};
  vga_init_printer(25, 80);
  init_TITLE_CARD_allocator();
  for (size_t i = 0;; i = (i + 1) % 10) {
    vga_printf("7 %x al:%d sz:%d\n", malloc_TITLE_CARD(sz[i % 5], al[i % 3]), al[i % 3], sz[i % 5]);
    // vga_printf("%d %d ", i, i);
  }
}
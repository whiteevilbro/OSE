#include "vga.h"

#include "memmgnt.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

static VGAChar* const VGA_TEXT_BASE = (VGAChar* const) 0xB8000;
size_t page_columns                 = 80;
size_t page_rows                    = 25;
static VGAChar* vga_buffer          = NULL;
static bool buffer_sync;

void vga_init_printer(const size_t rows, const size_t columns) {
  page_rows    = rows;
  page_columns = columns;
  vga_buffer   = malloc_immortal(rows * columns * sizeof(VGAChar), 0);
  vga_clear_screen();
}

void vga_flush(void) {
  if (!buffer_sync)
    memmove(VGA_TEXT_BASE, vga_buffer, page_rows * page_columns * (sizeof(VGAChar)));
  buffer_sync = true;
}

void vga_clear_screen(void) {
  buffer_sync = false;
  memzero(vga_buffer, page_rows * page_columns * sizeof(VGAChar));
  vga_flush();
}

void vga_print_char(const VGAChar c, const size_t x, const size_t y) {
  buffer_sync = false;

  vga_buffer[page_columns * y + x] = c;
}

void vga_copy(const Point dest, const Point src, const Point size) {
  vga_flush(); //todo rewrite without flush (order matters)
  VGAChar* dest_p = vga_buffer + dest.y * page_columns + dest.x;
  VGAChar* src_p  = VGA_TEXT_BASE + src.y * page_columns + src.x;

  VGAChar* dest_cursor;
  VGAChar* src_cursor;

  size_t length = dest.x + size.x;
  length        = min(length, page_columns) * sizeof(VGAChar);
  for (size_t i = 0; i < size.y; i++) {
    dest_cursor = dest_p + i * page_columns;
    src_cursor  = src_p + i * page_columns;
    memmove(dest_cursor, src_cursor, length);
  }
  buffer_sync = false;
}

void vga_fill(const Point dest, const Point size, VGAChar character) {
  VGAChar* dest_p = vga_buffer + dest.y * page_columns + dest.x;
  for (size_t i = 0; i < size.y; i++) {
    for (size_t j = 0; j < size.x; j++) {
      dest_p[i * page_columns + j] = character;
    }
  }
}

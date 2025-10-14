#include "vga.h"

#include "memmgnt.h"

#include <stdbool.h>

static VGA_charu* const VGA_TEXT_BASE = (VGA_charu*) 0xB8000;
static size_t COLUMNS = 80;
static size_t ROWS = 25;

static size_t row = 0;
static size_t column = 0;

void vga_init_printer(size_t rows, size_t columns) {
  ROWS = rows;
  COLUMNS = columns;
  row = 0;
  column = 0;
  vga_clear_screen();
}

void vga_clear_screen() {
  memzero(VGA_TEXT_BASE, ROWS * COLUMNS * sizeof(VGA_char));
}

void vga_print_char(VGA_charu c, size_t x, size_t y) {
  VGA_TEXT_BASE[COLUMNS * x + y] = c;
}

void vga_scroll_down() {
  memmove(VGA_TEXT_BASE, VGA_TEXT_BASE + COLUMNS, (ROWS - 1) * COLUMNS * sizeof(VGA_char));
  memzero(VGA_TEXT_BASE + (ROWS - 1) * COLUMNS, COLUMNS * sizeof(VGA_char));
}

void vga_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vga_vprintf(fmt, args);
  va_end(args);
}

void write(const char c) {
  switch (c) {
    case '\n':
      row++;
    case '\r':
      column = 0;
      break;
    default:
      vga_print_char((VGA_charu) {.repr = {.character = c, .fgcolor = WHITE, .bgcolor = BLACK}}, row, column++);
      row += column / COLUMNS;
      column %= COLUMNS;
      break;
  }
  if (row >= ROWS) {
    vga_scroll_down();
    row--;
  }
}

void vga_vprintf(const char* fmt, va_list args) {
  char c;
  bool parsing = false;
  char buf[32];
  size_t buf_i = 0;
  while ((c = *fmt++)) {
    if (parsing) {
      if (c == '%') {
        write(c);
        parsing = false;
        continue;
      }
      int tmp_int;
      const char* tmp_str;
      unsigned int tmp_uint;
      char tmp_char;
      switch (c) {
        case 'i':
        case 'd':
          tmp_int = va_arg(args, int);
          buf_i = 0;
          do {
            // write((char) (tmp_int % 10) + '0');
            buf[buf_i++] = (char) (tmp_int % 10) + '0';
            tmp_int /= 10;
          } while (tmp_int);
          while (buf_i--) {
            write(buf[buf_i]);
          }
          break;
        case 's':
          tmp_str = va_arg(args, const char*);
          while (*tmp_str) {
            write(*tmp_str++);
          }
          break;
        case 'x':
          tmp_uint = va_arg(args, unsigned int);
          buf_i = 0;
          do {
            tmp_char = (char) (tmp_uint & 0xf);
            tmp_char = (unsigned char) tmp_char > 9 ? tmp_char + 'a' - (char) 10 : tmp_char + '0';
            buf[buf_i++] = tmp_char;
            tmp_uint /= 16;
          } while (tmp_uint);
          while (buf_i--) {
            write(buf[buf_i]);
          }
          break;
        case 'c':
          tmp_int = va_arg(args, int);
          write((char) tmp_int);
          break;

        default:
          break;
      }
      parsing = false;
      continue;
    }
    if (c == '%') {
      parsing = true;
      continue;
    }
    write(c);
  }
}
#define _LIBC_LIMITS_H_
#include <assert.h>
#include <limits.h>
#include <memmgnt.h>
#include <stdbool.h>
#include <vga.h>

static void write(const char c);
static int parse_int(const char** const fmt);
static void write_int(int x, uint8_t flags, int width);
static void write_hex(unsigned int x, uint8_t flags, int width);
static void write_char(char c, uint8_t flags, int width);
static void write_rbuf(char* buf, size_t i, uint8_t flags, int width, char* prefix, size_t prefix_size);
static void write_str(const char* str, uint8_t flags, int width);

static VGAChar* const VGA_TEXT_BASE = (VGAChar*) 0xB8000;
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

void vga_clear_screen(void) {
  memzero(VGA_TEXT_BASE, ROWS * COLUMNS * sizeof(VGAChar));
}

void vga_print_char(VGAChar c, size_t x, size_t y) {
  VGA_TEXT_BASE[COLUMNS * x + y] = c;
}

void vga_scroll_down(void) {
  memmove(VGA_TEXT_BASE, VGA_TEXT_BASE + COLUMNS, (ROWS - 1) * COLUMNS * sizeof(VGAChar));
  memzero(VGA_TEXT_BASE + (ROWS - 1) * COLUMNS, COLUMNS * sizeof(VGAChar));
}

void vga_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vga_vprintf(fmt, args);
  va_end(args);
}

static void write(const char c) {
  switch (c) {
    case '\n':
      row++;
    case '\r':
      column = 0;
      break;
    default:
      vga_print_char((VGAChar) {.repr = {.character = c, .fgcolor = WHITE, .bgcolor = BLACK}}, row, column++);
      row += column / COLUMNS;
      column %= COLUMNS;
      break;
  }
  if (row >= ROWS) {
    vga_scroll_down();
    row--;
  }
}

static int parse_int(const char** const fmt) {
  int result = 0;
  char c;
  while ('0' <= (c = **fmt) && c <= '9') {
    result *= 10;
    result += c - '0';
    (*fmt)++;
  }
  return result;
}

enum {
  LEFT_FLAG = 0x1,
  SIGN_FLAG = 0x2,
  SPACE_FLAG = 0x4,
  TYPE_FLAG = 0x8,
  ZERO_FLAG = 0x10,
};

static void write_int(int x, uint8_t flags, int width) {
  char buf[(CHAR_BIT * sizeof(int) + 2) / 3]; // more than enough
  char prefix = 0;
  size_t buf_i = 0;

  bool neg = (width < 0);
  if (neg) {
    width = -width;
    prefix = '-';
  } else {
    if (flags & SIGN_FLAG) {
      prefix = '+';
    } else if (flags & SPACE_FLAG) {
      prefix = ' ';
    }
  }

  do {
    buf[buf_i++] = (char) (x % 10) + '0';
    x /= 10;
  } while (x);
  size_t prefix_size = prefix ? (size_t) 1 : (size_t) 0;
  if ((size_t) width >= prefix_size) {
    width -= (int) prefix_size;
  } else {
    width = 0;
  }
  write_rbuf(buf, buf_i, flags, width, &prefix, prefix_size);
}

static void write_hex(unsigned int x, uint8_t flags, int width) {
  char buf[(sizeof(int) * CHAR_BIT + 3) / 4]; // more than enogh
  char prefix[3];
  size_t pref_i = 0;
  size_t buf_i = 0;

  char case_mask = (flags & 0x80) ? 0 : 0x20;

  if (flags & SIGN_FLAG) {
    prefix[pref_i++] = '+';
  }
  if ((flags & TYPE_FLAG) && x) {
    prefix[pref_i++] = ('0');
    prefix[pref_i++] = ('X' | case_mask);
  }

  char tmp_char;
  do {
    tmp_char = (char) (x & 0xf);
    tmp_char = (unsigned char) tmp_char > 9 ? (tmp_char + 'A' - (char) 10) | case_mask : tmp_char + '0';
    buf[buf_i++] = tmp_char;
    x /= 16;
  } while (x);

  if ((size_t) width >= pref_i) {
    width -= (int) pref_i;
  } else {
    width = 0;
  }
  write_rbuf(buf, buf_i, flags, width, prefix, pref_i);
}

static void write_char(char c, uint8_t flags, int width) {
  write_rbuf(&c, 1, flags, width, NULL, 0);
}

// processes LEFT & ZERO flags
static void write_rbuf(char* buf, size_t i, uint8_t flags, int width, char* prefix, size_t prefix_size) {
  for (size_t j = 0; j < prefix_size; j++) {
    write(prefix[j]);
  }
  if ((size_t) width <= i || (flags & LEFT_FLAG)) {
    while (i--) {
      write(buf[i]);
    }
  }
  if ((size_t) width > i) {
    char c = flags & ZERO_FLAG ? '0' : ' ';
    for (size_t j = 0; j < (size_t) width - i; j++) {
      write(c);
    }
  }
  if (!(flags & LEFT_FLAG)) {
    while (i--) {
      write(buf[i]);
    }
  }
}

static void write_str(const char* str, uint8_t flags, int width) {
  size_t length = strlen(str);
  if ((size_t) width <= length || (flags & LEFT_FLAG)) {
    while (*str) {
      write(*str++);
    }
  }
  if ((size_t) width > length) {
    for (size_t j = 0; j < (size_t) width - length; j++) {
      write(' ');
    }
  }
  if (!(flags & LEFT_FLAG)) {
    while (*str) {
      write(*str++);
    }
  }
}

void vga_vprintf(const char* fmt, va_list args) {
  char c;

  enum {
    PARSING_NONE = false,
    PARSING_FLAGS = true,
    PARSING_WIDTH,
    PARSING_PRECISION,
    PARSING_LENGTH,
    PARSING_SPECIFIER
  } parsing = false;

  // avl:3 '0':1 '#':1 ' ':1 '+':1 '-':1
  uint8_t flags;
  int width = 0;
  while ((c = *fmt)) {
    switch (parsing) {
      case PARSING_FLAGS:
        switch (c) {
          case '-':
            flags |= LEFT_FLAG;
            break;
          case '+':
            flags |= SIGN_FLAG;
            break;
          case ' ':
            flags |= SPACE_FLAG;
            break;
          case '#':
            flags |= TYPE_FLAG;
            break;
          case '0':
            flags |= ZERO_FLAG;
            break;
          default:
            parsing = PARSING_WIDTH;
            continue;
        }
        fmt++;
        break;
      case PARSING_WIDTH:
        if (c == '*') {
          width = va_arg(args, int);
          if (width < 0)
            width = 0;
        } else if ('0' <= c && c <= '9') {
          width = parse_int(&fmt);
        } else {
          parsing = PARSING_PRECISION;
          continue;
        }
        break;
      case PARSING_PRECISION:
        parsing = PARSING_LENGTH;
        // break;
      case PARSING_LENGTH:
        parsing = PARSING_SPECIFIER;
        // break;
      case PARSING_SPECIFIER:
        switch (c) {
          case 'i':
          case 'd':
            write_int(va_arg(args, int), flags, width);
            break;
          case 's': //todo clear flags
            write_str(va_arg(args, const char*), flags, width);
            break;
          case 'X':
            flags |= 0x80;
            // FALLTHROUGH
          case 'x':
            write_hex(va_arg(args, unsigned int), flags, width);
            break;
          case 'c':
            write_char((char) va_arg(args, int), flags, width);
            break;

          default:
            break;
        }
        fmt++;
        parsing = false;
        continue;
      case PARSING_NONE:
        fmt++;
        if (c == '%') {
          if (*fmt != 'c') {
            parsing = true;
            flags = 0;
            // width = 0;
            continue;
          }
          fmt++;
        }
        write(c);
    }
  }
}
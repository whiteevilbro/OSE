#ifndef VGA_H_
#define VGA_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  BLACK = 0,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,
  MAGENTA = 5,
  BROWN = 6,
  LIGHT_GRAY = 7,
  DARK_GRAY = 8,
  LIGHT_BLUE = 9,
  LIGHT_GREEN = 10,
  LIGHT_CYAN = 11,
  LIGHT_RED = 12,
  LIGHT_MAGENTA = 13,
  YELLOW = 14,
  WHITE = 15
} VGA_color;

#pragma pack(push, 1)

typedef struct VGA_char {
  char character : 8;
  VGA_color fgcolor : 4;
  VGA_color bgcolor : 4;
} VGACharS;

typedef union {
  VGACharS repr;
  uint16_t attrchar;
} VGAChar;

#pragma pack(pop)

void vga_print_char(VGAChar c, size_t x, size_t y);
void vga_vprintf(const char* fmt, va_list args);
void vga_printf(const char* fmt, ...);
void vga_clear_screen(void);
void vga_init_printer(size_t rows, size_t columns);

#endif
#include "stddef.h"
#include "stdint.h"

extern void halt();

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

#pragma pack(push, 2)

typedef struct VGA_char {
  char character : 8;
  VGA_color fgcolor : 4;
  VGA_color bgcolor : 4;
} VGA_char;

typedef union {
  VGA_char repr;
  uint16_t attrchar;
} VGA_charu;

#pragma pack(pop)

volatile VGA_charu* const text_frame_buffer = (VGA_charu*) 0xB8000;

void kernel_entry(uint8_t columns, uint8_t row, uint8_t column) {
  char string[] = " Not so light-weight \"Hello, world!\" ";
  volatile VGA_charu* write_buf = text_frame_buffer + columns * row + column;
  for (size_t i = 0; i < sizeof(string) - 1; i++) {
    write_buf[i].repr = (VGA_char) {.bgcolor = WHITE, .fgcolor = BLACK, .character = string[i]};
  }
  halt();
}
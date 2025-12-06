#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "drivers/vga/vga.h"

#include <stdarg.h>
#include <stddef.h>

#define STDOUT_BUFFER_SIZE 512

typedef struct {
  VGA_color foreground : 4;
  VGA_color background : 4;
} Format;

typedef struct {
  char* buffer;
  size_t buffer_pos;
  size_t buffer_size;

  Point position;
  Point size;
  Point cursor;
  Format default_format; // would be nice to have it const, but it needed to be written on creation;
  Format format;
} Console;

#define EOF ((char) -1)

extern Console fullscreen;
extern Console* stdout;

void init_consoles(void);
void clear_console(Console* console);

int cflush(Console* console);

int cprintf(Console* console, const char* format, ...);
int printf(const char* format, ...);
int sprintf(char* s, const char* format, ...);
int snprintf(char* s, size_t n, const char* format, ...);

int vcprintf(Console* console, const char* format, va_list arg);
int vprintf(const char* format, va_list arg);
int vsprintf(char* s, const char* format, va_list arg);
int vsnprintf(char* s, size_t n, const char* format, va_list arg);

int cputc(int character, Console* console);
int cputs(const char* s, Console* const console);
// int putc(int character, Console* console);
#define putc(character, console) cputc(character, console)
int putchar(int character);
int puts(const char* s);

#endif

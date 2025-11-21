#include "assert.h"

#include "interrupts.h"
#include "vga.h"

extern void halt();

void vkernel_panic(const char* fmt, va_list args) {
  cli();
  vga_printf(fmt, args);
  halt();
}

void kernel_panic(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vkernel_panic(fmt, args);
}
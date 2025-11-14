#include "assert.h"

#include "interrupts.h"
#include "utils.h"
#include "vga.h"

void vkernel_panic(const char* fmt, va_list args) {
  cli();
  vga_vprintf(fmt, args);
  halt();
}

void kernel_panic(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vkernel_panic(fmt, args);
}

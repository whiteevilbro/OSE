#include "assert.h"

#include "console.h" // IWYU pragma: keep
#include "interrupts.h"

#include <stdarg.h>
#include <stddef.h>

noret vkernel_panic(const char* fmt, va_list args) {
  cli();
  stdout = &fullscreen;
  // clear_console(stdout);
  vprintf(fmt, args);
  halt();
  __builtin_unreachable();
}

noret kernel_panic(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vkernel_panic(fmt, args);
  va_end(args);
  __builtin_unreachable();
}

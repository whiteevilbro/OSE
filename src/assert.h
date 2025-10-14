#ifndef ASSERT_H_
#define ASSERT_H_

#include <stdarg.h>

#ifdef DEBUG
  #define assert(COND) (COND) && kernel_panic()
#else
  #define assert()
#endif

void vkernel_panic(const char* fmt, va_list args);
void kernel_panic(const char* fmt, ...);

#endif
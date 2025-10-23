#ifndef ASSERT_H_
#define ASSERT_H_

#include <stdarg.h>
#include <utils.h>

#ifdef DEBUG
  #define assert(COND) (COND) ? (void) (0) : kernel_panic(__FILE__ ":" __stringify2(__LINE__) " assertion (" #COND ") failed")
#else
  #define assert()
#endif

void vkernel_panic(const char* fmt, va_list args);
void kernel_panic(const char* fmt, ...);

#endif
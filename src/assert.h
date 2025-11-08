#ifndef ASSERT_H_
#define ASSERT_H_

#include <stdarg.h>

#ifdef DEBUG
  #define __stringify(x) #x
  #define __stringify2(x) __stringify(x)

  #define assert(COND) (COND) ? (void) (0) : kernel_panic(__FILE__ ":" __stringify2(__LINE__) " assertion (" #COND ") failed")
#else
  #define assert(COND)
#endif

void vkernel_panic(const char* fmt, va_list args);
void kernel_panic(const char* fmt, ...);

#endif

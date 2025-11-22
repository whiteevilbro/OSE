#ifndef ASSERT_H_
#define ASSERT_H_

#include "utils.h"

#include <stdarg.h>

#ifdef _DEBUG
  #define str(x) #x
  #define xstr(x) str(x)

  #define assert(COND) \
    (COND) ? (void) (0) : kernel_panic(__FILE__ ":" xstr(__LINE__) " assertion (" #COND ") failed")
#else
  #define assert(COND)
#endif

noret vkernel_panic(const char* fmt, va_list args);
noret kernel_panic(const char* fmt, ...);

#endif

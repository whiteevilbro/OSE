#ifndef STDARG_H_
#define STDARG_H_

#include <memmgnt.h>
#include <stdint.h>

#define va_list uint8_t*

#define va_start(v, l) v = ((size_t) ((uint8_t*) &l + sizeof(l) + 3) & (size_t) ~0b11)
#define va_arg(v, l) \
  *((l*) v);         \
  v = ((size_t) (v + sizeof(l) + 3) & (size_t) ~0b11)
#define va_end(v)
#define va_copy(dest, src) dest = src


#endif
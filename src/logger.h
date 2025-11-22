#ifndef LOGGER_H_
#define LOGGER_H_

#include "vga.h" // IWYU pragma: export

#ifdef _DEBUG
  #define DEBUG(...) vga_printf("[DEBUG] " __VA_ARGS__)
#else
  #define DEBUG(...)
#endif

#define INFO(...) vga_printf("[INFO] " __VA_ARGS__)
#define WARNING(...) vga_printf("[WARINING] " __VA_ARGS__)
#define ERROR(...) vga_printf("[ERROR] " __VA_ARGS__)

#endif

#ifndef LOGGER_H_
#define LOGGER_H_

#include "console.h" // IWYU pragma: export

#ifdef _DEBUG
  #define DEBUG(fmt, ...) printf("%s " fmt, "[\x1b[92mDEBUG\x1b[0m] ", __VA_ARGS__)
#else
  #define DEBUG(...)
#endif

#define INFO(fmt, ...) printf("%s " fmt, "[\x1b[32mINFO\x1b[0m] ", __VA_ARGS__)
#define WARNING(fmt, ...) printf("%s " fmt, "[\x1b[93mWARNING\x1b[0m] ", __VA_ARGS__)
#define ERROR(fmt, ...) printf("%s " fmt, "[\x1b[91mERROR\x1b[0m] ", __VA_ARGS__)

#endif

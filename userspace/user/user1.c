#include "../std/std.h" // IWYU pragma: keep

#include <stddef.h>

void main(int argc, const char** argv) {
  for (size_t i = 0; i < (size_t) argc; i++) {
    printf("%s\n", argv[i]);
  }
  return;
}

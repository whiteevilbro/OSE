#include <interrupts.h>
#include <vga.h>

extern void exp(void);

void kernel_entry(void) {
  vga_init_printer(25, 80);
  init_immortal_allocator();
  init_exceptions();
  exp();
}
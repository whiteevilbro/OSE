#include "experiments.h"
#include "interrupts.h"
#include "memmgnt.h"
#include "utils.h"
#include "vga.h"

void kernel_entry(void) {
  vga_init_printer(25, 80);
  init_immortal_allocator();
  init_interrupts();
  experiment();
  halt();
}
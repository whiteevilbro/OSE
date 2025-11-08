#include "experiments.h"
#include "interrupts.h"
#include "memmgnt.h"
#include "pit.h"
#include "utils.h"
#include "vga.h"

#include <stdbool.h>

void kernel_entry(void) {
  vga_init_printer(25, 80);
  init_immortal_allocator();
  init_interrupts();

  init_pic(AUTOMATIC_EOI);
  init_timer(50);
  enable_io_devices(SYSTEM_TIMER);
  sti();
  uint32_t m = 0;
  while (true) {
    if (millis - m > 1000) {
      vga_printf("%d ", millis - m);
      m = millis;
    }
  }

  experiment();
  halt();
}

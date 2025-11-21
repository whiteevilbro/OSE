#include "experiments.h"
#include "interrupts.h"
#include "memmgnt.h"
#include "pit.h"
#include "utils.h"
#include "vga.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void kernel_entry(void) {
  vga_init_printer(25, 80);
  init_immortal_allocator();
  init_interrupts();

  init_pic(AUTOMATIC_EOI);
  init_timer(1000);
  enable_io_devices(SYSTEM_TIMER);
  sti();
  int8_t s    = -10;
  int32_t m   = 0;
  uint32_t mi = 0;
  while (true) {
    if (millis - mi > 1000) {
      mi = millis;
      s++;
      m += s / 60;
      s %= 60;
      vga_printf("%02d:%02d\r", m, s);
    }
  }

  experiment();
  halt();
}

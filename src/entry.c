#include "acpi.h"
#include "drivers/ps2/ps2_controller.h"
#include "drivers/ps2/ps2_device_general.h"
#include "experiments.h"
#include "interrupts.h"
#include "memmgnt.h"
#include "pit.h"
#include "utils.h"
#include "vga.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void kernel_entry(const void* memsize) {
  vga_init_printer(25, 80);
  init_immortal_allocator();
  init_interrupts();
  init_acpi((void*) (((size_t) memsize) << 10));

  init_pic(AUTOMATIC_EOI);
  init_timer(40000);
  enable_io_devices(SYSTEM_TIMER);
  sti();

  //todo make send_data resend data (commands too)
  init_ps2_controller(); //todo logging inside there
  ps2_reset_devices();
  ps2_detect_devices();


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

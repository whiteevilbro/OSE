#include "acpi.h"
#include "console.h"
#include "drivers/ps2/ps2_controller.h"
#include "drivers/ps2/ps2_device_general.h"
#include "drivers/ps2/ps2_keyboard.h"
#include "experiments.h" // IWYU pragma: keep
#include "interrupts.h"
#include "memmgnt.h"
#include "pit.h"
#include "utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void kernel_entry(const void* memsize) {
  init_acpi((void*) (((size_t) memsize) << 10));
  init_immortal_allocator();
  init_consoles();
  init_interrupts();

  init_pic(AUTOMATIC_EOI);
  init_timer(200);
  enable_io_devices(SYSTEM_TIMER);
  sti();

  // //todo make send_data resend data (commands too)
  init_ps2_controller(); //todo logging inside there
  ps2_reset_devices();
  ps2_detect_devices();

  if (devices[0] == MF_KEYBOARD || devices[0] == MF_KEYBOARD1) {
    init_ps2_keyboard(0);
  }

  void* stack = (uint8_t*) malloc_immortal(4 << 10, 8) + (size_t) (4 << 10);
  jump_to_userspace(experiment(10), stack);

  halt();
}

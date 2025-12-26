#include "acpi.h"
#include "console.h"
#include "drivers/ps2/ps2_controller.h"
#include "drivers/ps2/ps2_device_general.h"
#include "drivers/ps2/ps2_keyboard.h"
#include "interrupts.h"
#include "memmgnt.h"
#include "paging.h"
#include "pit.h"
#include "processes/process.h"
#include "syscalls.h"
#include "utils.h"
#include "vga.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


const char* params[] = {"Hello, world!", "In userspace", "Passed as argv argument", "Not in 14 bytes though :("};

void kernel_entry(const void* memsize) {
  init_acpi((void*) (((size_t) memsize) << 10));
  init_immortal_allocator();
  init_consoles();

  Console* kernel_console = malloc_immortal(sizeof(Console), 0);
  *kernel_console         = (Console){
              .buffer         = malloc_immortal(STDOUT_BUFFER_SIZE, 8),
              .buffer_pos     = 0,
              .buffer_size    = STDOUT_BUFFER_SIZE,
              .position       = (Point){.x = 0, .y = 0},
              .size           = (Point){.x = 39, .y = 12},
              .cursor         = (Point){.x = 0, .y = 0},
              .default_format = fullscreen.format,
              .format         = fullscreen.default_format,
  };
  clear_console(kernel_console);

  stdout = kernel_console;
  init_interrupts();

  init_pic(AUTOMATIC_EOI);
  init_timer(18);
  enable_io_devices(SYSTEM_TIMER);
  sti();

  // //todo make send_data resend data (commands too)
  init_ps2_controller(); //todo logging inside there
  ps2_reset_devices();
  ps2_detect_devices();

  if (devices[0] == MF_KEYBOARD || devices[0] == MF_KEYBOARD1) {
    init_ps2_keyboard(0);
  }

  setup_kernel_paging();
  init_scheduler();
  register_syscalls();

  Console user_console = (Console){
      .buffer         = malloc_immortal(STDOUT_BUFFER_SIZE, 0),
      .buffer_pos     = 0,
      .buffer_size    = STDOUT_BUFFER_SIZE,
      .position       = (Point){.x = 0, .y = 12},
      .size           = (Point){.x = 25, .y = 12},
      .cursor         = (Point){.x = 0, .y = 0},
      .default_format = (Format){.background = BROWN, .foreground = WHITE},
      .format         = (Format){.background = BROWN, .foreground = WHITE},
  };
  clear_console(&user_console);

  enqueue_process((void (*)(void)) 0x20000, 4, params, &user_console);

  user_console.size = (Point){.x = 15, .y = 12};

  user_console.buffer         = malloc_immortal(STDOUT_BUFFER_SIZE, 0);
  user_console.position       = (Point){.x = 25, .y = 12};
  user_console.default_format = (Format){.background = LIGHT_RED, .foreground = WHITE};
  user_console.format         = user_console.default_format;
  clear_console(&user_console);
  enqueue_process((void (*)(void)) 0x30000, 0, NULL, &user_console);

  user_console.size = (Point){.x = 40, .y = 12};

  user_console.buffer         = malloc_immortal(STDOUT_BUFFER_SIZE, 0);
  user_console.position       = (Point){.x = 40, .y = 0};
  user_console.default_format = (Format){.background = LIGHT_GREEN, .foreground = BLACK};
  user_console.format         = user_console.default_format;
  clear_console(&user_console);
  enqueue_process((void (*)(void)) 0x40000, 0, NULL, &user_console);


  user_console.buffer         = malloc_immortal(STDOUT_BUFFER_SIZE, 0);
  user_console.position       = (Point){.x = 40, .y = 12};
  user_console.default_format = (Format){.background = LIGHT_BLUE, .foreground = WHITE};
  user_console.format         = user_console.default_format;
  clear_console(&user_console);
  enqueue_process((void (*)(void)) 0x50000, 0, NULL, &user_console);

  handlerTable[SYSTEM_TIMER_VECTOR] = (InterruptHandler) save_and_switch_process;
  switch_process();

  halt();
}

#include "experiments.h"

#include "console.h" // IWYU pragma: keep
#include "interrupts.h"
#include "user_calls.h" // IWYU pragma: keep
#include "utils.h"

#include <stddef.h>
#include <stdint.h>

#if __GNUC__
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wunused-variable"
  #pragma GCC diagnostic ignored "-Wunused-function"
#endif

static void user_process(void);

extern void hack(void);

static void clr(void);
static void scall(const Context* const ctx);

int globali = 0;

// typedef? Never heard of 'em
void (*experiment(int n))(void) {
  disable_io_devices(SYSTEM_TIMER);
  switch (n) {
    case 0:
      // set_interrupt_handler(0x30, INTERRUPT_GATE, USER_PL, scall);
      set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, KERNEL_PL, (InterruptHandler) clr);
      enable_io_devices(SYSTEM_TIMER);
      return hack;
    case 1:
      return user_process;
    default:
      return halt;
  }
}

static void clr(void) {
  clear_console(stdout);
}

int param;

extern void nrec(int param);

static void user_process(void) {
  // switch (param % 4) {
  //   case 0:
  //     exit(param);
  //     return;
  //   case 1:
  //     *(uint8_t*) 0x42 = 0;
  //     return;
  //   case 2:
  //     exp();
  //     return;
  //   case 3:
  //     *(uint8_t*) 0x900000 = 0;
  //     return;
  // }
  nrec(param);
  exit(param);
}

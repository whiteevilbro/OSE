#include "experiments.h"

#include "console.h" // IWYU pragma: keep
#include "interrupts.h"
#include "utils.h"

#include <stddef.h>
#include <stdint.h>

#if __GNUC__
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wunused-variable"
  #pragma GCC diagnostic ignored "-Wunused-function"
#endif

static void experiment1(void);
static void experiment2(void);
static void experiment3(void);
static void experiment4(void);
static void experiment5(void);
static void experiment6(void);
static void experiment7(void);
static void experiment8(void);
static void experiment9(void);
static void experiment10(void);

extern void hack(void);

static void clr(void);
static void zero_global(void);
static void empty(void);
static void scall(const Context* const ctx);

int globali = 0;

// typedef? Never heard of 'em
void (*experiment(int n))(void) {
  disable_io_devices(SYSTEM_TIMER);
  switch (n) {
    case 0:
      set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, KERNEL_PL, (InterruptHandler) clr);
      enable_io_devices(SYSTEM_TIMER);
      return hack;
    case 1:
      return experiment1;
    case 2:
      return experiment2;
    case 3:
      return experiment3;
    case 4:
      return experiment4;
    case 5:
      return experiment5;
    case 6:
      //! need to lower timer frequency
      set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, KERNEL_PL, (InterruptHandler) experiment4);
      enable_io_devices(SYSTEM_TIMER);
      return experiment6;
    case 7:
      set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, KERNEL_PL, (InterruptHandler) zero_global);
      enable_io_devices(SYSTEM_TIMER);
      return experiment7;
    case 8:
      globali = 1;
      set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, KERNEL_PL, (InterruptHandler) zero_global);
      enable_io_devices(SYSTEM_TIMER);
      return experiment8;
    case 9:
      //! umm...
      set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, KERNEL_PL, (InterruptHandler) empty);
      enable_io_devices(SYSTEM_TIMER);
      return experiment9;
    case 10:
      set_interrupt_handler(0x30, INTERRUPT_GATE, USER_PL, scall);

      set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, KERNEL_PL, (InterruptHandler) zero_global);
      enable_io_devices(SYSTEM_TIMER);
      return experiment10;
    default:
      return halt;
  }
}

static void experiment1(void) {
  halt();
}

static void experiment2(void) {
  printf("1\n");
  halt();
}

static void experiment3(void) {
  for (;;)
    printf("%d ", globali++);
}

static void experiment4(void) {
  size_t esp;
  __asm__ __volatile__("mov %0, esp" : "=g"(esp));
  printf("%#010x ", esp);
  halt();
}

static void experiment5(void) {
  cli();
  halt();
}

static void experiment6(void) {
  experiment3();
}

static void experiment7(void) {
  experiment3();
}

static void experiment8(void) {
  for (; globali;)
    printf("%d ", globali++);
  printf("\nGOTCHA!\n");
  halt();
}

static void experiment9(void) {
  ((uint8_t*) gdt)[12] = 0;
  ((uint8_t*) gdt)[13] = 0;
  ((uint8_t*) gdt)[14] = 0;
}

static void experiment10(void) {
  exp();
}

static void clr(void) {
  clear_console(stdout);
}

static void scall(const Context* const ctx) {
  printf("%d ", ctx->eax);
}

static void zero_global(void) {
  globali = 0;
}

static void empty(void) {
  return;
}

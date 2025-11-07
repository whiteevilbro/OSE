#include "experiments.h"

#include "assert.h"
#include "interrupts.h"
#include "utils.h"
#include "vga.h"

#if __GNUC__
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#define PANIC_MESSAGE "Kernel panic: unhandled interrupt #%#010x at %#010x:%#010x\n\n" \
                      "Registers:\n"                                                   \
                      "  EAX: %#010x, ECX: %#010x, EDX: %#010x, EBX: %#010x\n"         \
                      "  ESP: %#010x, EBP: %#010x, ESI: %#010x, EDI: %#010x\n\n"       \
                      "EFLAGS:\n  %#010x\n"

#define UNPACK_CONTEXT(ctx) ctx->vector, ctx->cs, ctx->eip,         \
                            ctx->eax, ctx->ecx, ctx->edx, ctx->ebx, \
                            ctx->esp, ctx->ebp, ctx->esi, ctx->edi, \
                            ctx->eflags

static int global = 0;

static void timer_handler(const Context* const ctx) {
  // kernel_panic(PANIC_MESSAGE, UNPACK_CONTEXT(ctx));
  // vga_printf("%d ", global++);
  // if (global < 10) {
  // send_EOI_master();
  //   sti();
  // }
  // halt();
  // global = 0;

  disable_io_devices(SYSTEM_TIMER);
  for (size_t i = 0; i < 5; i++) {
    for (size_t j = 0; j < 1000000; j++) {
      outb(0x80, 0xff);
    }
    vga_printf("%d ", i);
  }
  vga_printf("\n");

  sti();

  for (size_t i = 0; i < 5; i++) {
    for (size_t j = 0; j < 1000000; j++) {
      outb(0x80, 0xff);
    }
    vga_printf("%d ", i);
  }
}

static void keyboard_handler(const Context* const ctx) {
  // kernel_panic(PANIC_MESSAGE, UNPACK_CONTEXT(ctx));
  uint8_t code;
  inb(0x60, code);
  vga_printf("%d ", code);
  // send_EOI_master();

  // sti();

  halt();
}

void experiment(void) {
  init_pic(AUTOMATIC_EOI);
  // init_pic(NON_AUTOMATIC_EOI);

  set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, &timer_handler);
  // set_interrupt_handler(SYSTEM_TIMER_VECTOR, TRAP_GATE, &timer_handler);

  set_interrupt_handler(KEYBOARD_VECTOR, INTERRUPT_GATE, &keyboard_handler);
  // set_interrupt_handler(KEYBOARD_VECTOR, TRAP_GATE, &keyboard_handler);

  enable_io_devices(SYSTEM_TIMER);
  enable_io_devices(KEYBOARD);

  sti();

  halt();
  // for (;;) {
  //   vga_printf("%d ", global++);
  // }
}
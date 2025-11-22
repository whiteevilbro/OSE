#include "interrupts.h"

#include "assert.h"
#include "ports.h"

#include <stdbool.h>

#define lidt(p) __asm__ __volatile__("lidt %0" ::"m"(p));

#define VECTOR_COUNT 256
#define EXCEPTION_COUNT 0x20
#define TRAMPOLINE_COUNT VECTOR_COUNT
#define TRAMPOLINE_SIZE 8

#pragma pack(push, 1)

typedef struct {
  uint16_t limit;
  uint32_t base;
} IDTPseudoDescriptor;

#pragma pack(pop)

GateDescriptor* idt;
static InterruptHandler* handlerTable;

extern void collect_ctx(void);

static bool has_error_code(uint8_t vector);
static void* generate_trampolines(void);
static void write_descriptor(GateDescriptor* const desc,
                             const void* const trampoline,
                             GateDescriptorType type, bool present);
void universal_interrupt_handler(const Context* const ctx);
void init_interrupts(void);
static void generate_idt(void);
static inline void init_exceptions(void);
void init_pic(EOIType automatic_EOI);
void disable_io_devices(IODevice devices);
void enable_io_devices(IODevice devices);

#define PANIC_MESSAGE                                              \
  "Kernel panic: unhandled interrupt #%#010x at %#010x:%#010x\n\n" \
  "Registers:\n"                                                   \
  "  EAX: %#010x, ECX: %#010x, EDX: %#010x, EBX: %#010x\n"         \
  "  ESP: %#010x, EBP: %#010x, ESI: %#010x, EDI: %#010x\n\n"       \
  "EFLAGS:\n  %#010x\n"

#define UNPACK_CONTEXT(ctx)                                               \
  ctx->vector, ctx->cs, ctx->eip, ctx->eax, ctx->ecx, ctx->edx, ctx->ebx, \
      ctx->esp, ctx->ebp, ctx->esi, ctx->edi, ctx->eflags

void universal_interrupt_handler(const Context* const ctx) {
  InterruptHandler handler = handlerTable[ctx->vector];
  if (handler) {
    handler(ctx);
    return;
  }

  if (has_error_code(ctx->vector))
    kernel_panic(PANIC_MESSAGE "\nError code: %#010x", UNPACK_CONTEXT(ctx),
                 ctx->error_code);
  else
    kernel_panic(PANIC_MESSAGE, UNPACK_CONTEXT(ctx));
}

static bool has_error_code(uint8_t vector) {
  switch (vector) {
    case 0x08:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x11:
    case 0x15:
    case 0x1D:
    case 0x1E:
      return true;
    default:
      return false;
  }
}

static void* generate_trampolines(void) {
  uint8_t* trampolines = malloc_immortal(TRAMPOLINE_COUNT * TRAMPOLINE_SIZE, 8);
  uint8_t* trampoline  = trampolines;
  uint8_t f            = 0;
  for (size_t vector = 0; vector < TRAMPOLINE_COUNT; vector++) {
    if (!(f = has_error_code((uint8_t) vector)))
      *trampoline++ = 0x0E;           // push cs
    *trampoline++ = 0x6A;             // push imm8
    *trampoline++ = (uint8_t) vector; // vector imm8

    *trampoline++ = 0xE9; // jmp
    *((uint32_t*) trampoline) =
        (uint32_t) ((size_t) &collect_ctx - (size_t) trampoline -
                    4); // collect_ctx
    trampoline += 4;
    trampoline += f;
  }
  assert(trampoline <= trampolines + TRAMPOLINE_COUNT * TRAMPOLINE_SIZE);

  return trampolines;
}

static void write_descriptor(GateDescriptor* const desc,
                             const void* const trampoline,
                             GateDescriptorType type, bool present) {
  *desc = (GateDescriptor) {
      .repr = {
          .offset_low                 = (uint16_t) ((size_t) trampoline & 0xFFFF),
          .segment_selector           = (uint16_t) CODE_SEGMENT,
          .type                       = type,
          .descriptor_privilege_level = KERNEL_PL,
          .present                    = present,
          .offset_high                = (uint16_t) (((size_t) trampoline >> 16) & 0xFFFF),

          .reserved = 0,
          .fixed1   = 0,
          .fixed2   = 0,
      }};
}

void init_interrupts(void) {
  generate_idt();
  handlerTable = calloc_immortal(sizeof(InterruptHandler) * VECTOR_COUNT, 8);
  init_exceptions();
}

static void generate_idt(void) {
  void* trampolines = generate_trampolines();

  idt = malloc_immortal(sizeof(GateDescriptor) * VECTOR_COUNT, 8);
  for (size_t i = 0; i < VECTOR_COUNT; i++) {
    write_descriptor(idt + i,
                     (void*) ((uint8_t*) trampolines + TRAMPOLINE_SIZE * i),
                     INTERRUPT_GATE, false);
  }

  IDTPseudoDescriptor descriptor = {
      .limit = sizeof(GateDescriptor) * VECTOR_COUNT - 1,
      .base  = (uint32_t) idt};

  lidt(descriptor);
}

static inline void init_exceptions(void) {
  for (size_t i = 0; i < EXCEPTION_COUNT; i++) {
    idt[i].repr.present = 1;
  }
}

void set_interrupt_handler(uint8_t vector, GateDescriptorType type,
                           InterruptHandler handler) {
  handlerTable[vector]     = handler;
  idt[vector].repr.type    = type;
  idt[vector].repr.present = 1;
}

// ========= 8259 IO =========

#define PIC_SLAVE_IRQ_LINE 0x2

void init_pic(EOIType automatic_EOI) {
  union ICW1 {
    struct ICW1S {
      uint8_t ICW4_needed : 1;

      enum {
        CASCADE_MODE = 0x0,
        SINGLE_MODE  = 0x1,
      } sngl        : 1;

      uint8_t zero2 : 1;

      enum {
        EDGE_TRIGGERED_MODE  = 0x0,
        LEVEL_TRIGGERED_MOCE = 0x1,
      } trigger_mode : 1;

      uint8_t one    : 1;
      uint8_t zero1  : 4;
    } repr;

    uint8_t byte;
  } ICW1 = {.repr = {.trigger_mode = EDGE_TRIGGERED_MODE,
                     .sngl         = CASCADE_MODE,
                     .ICW4_needed  = 1,
                     .zero1        = 0,
                     .zero2        = 0,
                     .one          = 1}};

  union ICW2 {
    struct ICW2S {
      uint8_t zero   : 3;
      uint8_t vector : 5;
    } repr;

    uint8_t byte;
  } ICW2_master = {.repr = {.vector = PIC_MASTER_VECTOR_RANGE_START >> 3,
                            .zero   = 0}},
    ICW2_slave  = {
         .repr = {.vector = PIC_SLAVE_VECTOR_RANGE_START >> 3, .zero = 0}};

  uint8_t ICW3_master = 0x1 << PIC_SLAVE_IRQ_LINE;
  uint8_t ICW3_slave  = PIC_SLAVE_IRQ_LINE;

  union ICW4 {
    struct ICW4S {
      enum {
        MODE_MCS8x = 0x0,
        MODE_808x  = 0x1,
      } mode                : 1;

      EOIType automatic_EOI : 1;

      enum {
        NON_BUFFERED_MODE    = 0x0,
        BUFFERED_MODE_SLAVE  = 0x2,
        BUFFERED_MODE_MASTER = 0x3,
      } buffer_mode                  : 2;

      bool special_fully_nested_mode : 1;

      uint8_t zero                   : 3;
    } repr;

    uint8_t byte;
  } ICW4 = {.repr = {.automatic_EOI             = automatic_EOI,
                     .mode                      = MODE_808x,
                     .buffer_mode               = NON_BUFFERED_MODE,
                     .special_fully_nested_mode = false}};

  outb(PIC_MASTER_COMMAND_PORT, ICW1.byte);
  outb(PIC_SLAVE_COMMAND_PORT, ICW1.byte);
  pseudodelay();
  outb(PIC_MASTER_DATA_PORT, ICW2_master.byte);
  outb(PIC_SLAVE_DATA_PORT, ICW2_slave.byte);
  pseudodelay();
  outb(PIC_MASTER_DATA_PORT, ICW3_master);
  outb(PIC_SLAVE_DATA_PORT, ICW3_slave);
  pseudodelay();
  outb(PIC_MASTER_DATA_PORT, ICW4.byte);
  outb(PIC_SLAVE_DATA_PORT, ICW4.byte);
  pseudodelay();

  outb(PIC_MASTER_DATA_PORT, 0xff);
  outb(PIC_SLAVE_DATA_PORT, 0xff);
}

void enable_io_devices(IODevice devices) {
  uint8_t mask;
  if (devices & 0x100) {
    inb(PIC_SLAVE_DATA_PORT, mask);
    mask &= ~((uint8_t) devices);
    outb(PIC_SLAVE_DATA_PORT, mask);
  }
  if (!(devices & 0x100) || devices == ALL_DEVICES) {
    inb(PIC_MASTER_DATA_PORT, mask);
    mask &= ~((uint8_t) devices);
    outb(PIC_MASTER_DATA_PORT, mask);
  }
}

void disable_io_devices(IODevice devices) {
  uint8_t mask;
  if (devices & 0x100) {
    inb(PIC_SLAVE_DATA_PORT, mask);
    mask |= (uint8_t) devices;
    outb(PIC_SLAVE_DATA_PORT, mask);
  }
  if (!(devices & 0x100) || devices == ALL_DEVICES) {
    inb(PIC_MASTER_DATA_PORT, mask);
    mask |= (uint8_t) devices;
    outb(PIC_MASTER_DATA_PORT, mask);
  }
}

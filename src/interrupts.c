#include <assert.h>
#include <interrupts.h>
#include <memmgnt.h>
#include <stdbool.h>
#include <utils.h>

#define lidt(p) __asm__ __volatile__("lidt %0" ::"m"(p));

#define VECTOR_COUNT 256
#define TRAMPOLINE_COUNT VECTOR_COUNT
#define TRAMPOLINE_SIZE 8

typedef enum {
  INTERRUPT_GATE = 0xE,
  TRAP_GATE = 0xF,
} GateDescriptorType;

#pragma pack(push, 1)

typedef struct {
  uint16_t offset_low : 16;
  uint16_t segment_selector : 16;
  uint8_t reserved : 5;
  uint8_t fixed1 : 3;
  GateDescriptorType type : 4;
  uint8_t fixed2 : 1;
  PrivilegeLevel descriptor_privilege_level : 2;
  uint8_t present : 1;
  uint16_t offset_high : 16;
} GateDescriptorS;

typedef union {
  GateDescriptorS repr;
  uint64_t qword;
} GateDescriptor;

typedef struct {
  uint16_t limit;
  uint32_t base;
} IDTPseudoDescriptor;

#pragma pack(pop)

typedef struct a {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax; // 32

  alignas(4) uint16_t gs;
  alignas(4) uint16_t fs;
  alignas(4) uint16_t es;
  alignas(4) uint16_t ds; // 16

  alignas(4) uint8_t vector; // 4
  uint32_t error_code;
  uint32_t eip;
  alignas(4) uint16_t cs;
  uint32_t eflags; // 16
} Context;

static GateDescriptor* idt;

extern void collect_ctx(void);

static bool has_error_code(uint8_t vector);
static void* generate_trampolines(void);
static void write_descriptor(GateDescriptor* const desc, const void* const trampoline);
void universal_interrupt_handler(const Context* const ctx);
void init_exceptions(void);

#define PANIC_MESSAGE "Kernel panic: unhandled interrupt #%#010x at %#010x:%#010x\n\n" \
                      "Registers:\n"                                                   \
                      "  EAX: %#010x, ECX: %#010x, EDX: %#010x, EBX: %#010x\n"         \
                      "  ESP: %#010x, EBP: %#010x, ESI: %#010x, EDI: %#010x\n\n"       \
                      "EFLAGS:\n  %#010x\n"

#define UNPACK_CONTEXT(ctx) ctx->vector, ctx->cs, ctx->eip,         \
                            ctx->eax, ctx->ecx, ctx->edx, ctx->ebx, \
                            ctx->esp, ctx->ebp, ctx->esi, ctx->edi, \
                            ctx->eflags

void universal_interrupt_handler(const Context* const ctx) {
  if (has_error_code(ctx->vector))
    kernel_panic(PANIC_MESSAGE "\nError code: %#010x", UNPACK_CONTEXT(ctx), ctx->error_code);
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
  uint8_t* trampoline = trampolines;
  uint8_t f = 0;
  for (size_t vector = 0; vector < TRAMPOLINE_COUNT; vector++) {
    if (!(f = has_error_code((uint8_t) vector)))
      *trampoline++ = 0x0E; // push cs
    // else
    //   *trampoline++ = 0x90;
    *trampoline++ = 0x6A;             // push imm8
    *trampoline++ = (uint8_t) vector; // vector imm8

    *trampoline++ = 0xE9;                                                                     // jmp
    *((uint32_t*) trampoline) = (uint32_t) ((size_t) &collect_ctx - (size_t) trampoline - 4); // collect_ctx
    trampoline += 4;
    trampoline += f;
  }
  assert(trampoline <= trampolines + TRAMPOLINE_COUNT * TRAMPOLINE_SIZE);

  return trampolines;
}

static void write_descriptor(GateDescriptor* const desc, const void* const trampoline) {
  *desc = (GateDescriptor) {
      .repr = {
          .offset_low = (uint16_t) ((size_t) trampoline & 0xFFFF),
          .segment_selector = (uint16_t) CODE_SEGMENT,
          .type = INTERRUPT_GATE,
          .descriptor_privilege_level = KERNEL_PL,
          .present = 1,
          .offset_high = (uint16_t) (((size_t) trampoline >> 16) & 0xFFFF),

          .reserved = 0,
          .fixed1 = 0,
          .fixed2 = 0,
      }};
}

void init_exceptions(void) {
  void* trampolines = generate_trampolines();

  idt = malloc_immortal(sizeof(GateDescriptor) * VECTOR_COUNT, 8);
  for (size_t i = 0; i < VECTOR_COUNT; i++) {
    write_descriptor(idt + i, (void*) ((uint8_t*) trampolines + TRAMPOLINE_SIZE * i));
  }

  IDTPseudoDescriptor descriptor = {.limit = sizeof(GateDescriptor) * (VECTOR_COUNT - 1) - 1, .base = (uint32_t) idt};

  lidt(descriptor);
}

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include "memmgnt.h"
#include "ports.h"
#include "utils.h"

#include <stdint.h>

#define cli() __asm__ __volatile__("cli" : : : "memory", "cc");
#define sti() __asm__ __volatile__("sti" : : : "memory", "cc");

#define EOI 0x20
#define PIC_MASTER_COMMAND_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_COMMAND_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_MASTER_VECTOR_RANGE_START 0x20
#define PIC_SLAVE_VECTOR_RANGE_START 0x28

#define send_EOI_master() outb(PIC_MASTER_COMMAND_PORT, EOI);
#define send_EOI_slave() outb(PIC_SLAVE_COMMAND_PORT, EOI)

#define PANIC_MESSAGE                                              \
  "Kernel panic: unhandled interrupt #%#010x at %#010x:%#010x\n\n" \
  "Registers:\n"                                                   \
  "  EAX: %#010x, ECX: %#010x, EDX: %#010x, EBX: %#010x\n"         \
  "  ESP: %#010x, EBP: %#010x, ESI: %#010x, EDI: %#010x\n\n"       \
  "EFLAGS:\n  %#010x\n"

#define ERROR_CODE_MESSAGE \
  "Error code: %#010x\n"

#define CR_MESSAGE       \
  "Control registers:\n" \
  "  CR0: %#010x\n"      \
  "  CR2: %#010x\n"      \
  "  CR3: %#010x\n"      \
  "  CR4: %#010x\n"

#define UNPACK_CONTEXT(ctx)                                               \
  ctx->vector, ctx->cs, ctx->eip, ctx->eax, ctx->ecx, ctx->edx, ctx->ebx, \
      ctx->esp, ctx->ebp, ctx->esi, ctx->edi, ctx->eflags

#define UNPACK_CR(crctx) \
  crctx.cr0, crctx.cr2, crctx.cr3, crctx.cr4

#if __GNUC__ >= 15
  #define pushfd()                   \
    uint32_t __eflags;               \
    __asm__ __volatile__("pushf\n\t" \
                         "pop %0" : "=r"(__eflags) : : "memory", "redzone")
  #define popfd()                      \
    __asm__ __volatile__("push %0\n\t" \
                         "popf" : : "=r"(__eflags) : "memory", "cc", "redzone")
#else
  #define pushfd()                   \
    uint32_t __eflags;               \
    __asm__ __volatile__("pushf\n\t" \
                         "pop %0" : "=r"(__eflags) : : "memory")
  #define popfd()                      \
    __asm__ __volatile__("push %0\n\t" \
                         "popf" : : "r"(__eflags) : "memory", "cc")
#endif

typedef enum {
  INTERRUPT_GATE = 0xE,
  TRAP_GATE      = 0xF,
} GateDescriptorType;

typedef enum {
  DIVISION_ERROR_VECTOR           = 0x00,
  DEBUG_VECTOR                    = 0x01,
  NMI_VECTOR                      = 0x02,
  BREAKPOINT_VECTOR               = 0x03,
  OVERFLOW_VECTOR                 = 0x04,
  BOUND_RANGE_EXCEEDED_VECTOR     = 0x05,
  INVALID_OPCODE_VECTOR           = 0x06,
  DEVICE_NOT_AVAILABLE_VECTOR     = 0x07,
  DOUBLE_FAULT_VECTOR             = 0x08,
  //
  INVALID_TSS_VECTOR              = 0x0A,
  SEGMENT_NOT_PRESENT_VECTOR      = 0x0B,
  STACK_SEGMENT_FAULT_VECTOR      = 0x0C,
  GENERAL_PROTECTION_FAULT_VECTOR = 0x0D,
  PAGE_FAULT_VECTOR               = 0x0E,
  // reserved
  X87_FLOATING_POINT_VECTOR       = 0x10,
  ALIGNMENT_CHECK_VECTOR          = 0x11,
  MACHINE_CHECK_VECTOR            = 0x12,
  SIMD_FLOATING_POINT_VECTOR      = 0x13,
  VIRTUALIZATION_VECTOR           = 0x14,
  CONTROL_PROTECTION_VECTOR       = 0x15,

  GP = GENERAL_PROTECTION_FAULT_VECTOR,
  PF = PAGE_FAULT_VECTOR,
} ExceptionVector;

typedef struct {
  size_t cr0;
  size_t cr2;
  size_t cr3;
  size_t cr4;
} CRContext;

typedef struct {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax; // 32

  Alignas(4) uint16_t gs;
  Alignas(4) uint16_t fs;
  Alignas(4) uint16_t es;
  Alignas(4) uint16_t ds; // 16

  Alignas(4) uint8_t vector; // 4
  uint32_t error_code;
  uint32_t eip;
  Alignas(4) uint16_t cs;
  uint32_t eflags; // 16
} Context;

typedef enum {
  SYSTEM_TIMER = 0x1,
  KEYBOARD     = 0x1 << 1,

  MOUSE         = (0x1 << 4) | 0x100,
  ATA_CHANNEL_1 = (0x1 << 6) | 0x100,

  FLOPPY = 0x1 << 6,

  ALL_DEVICES = 0xffff,
} IODevice;

typedef enum {
  SYSTEM_TIMER_VECTOR = PIC_MASTER_VECTOR_RANGE_START,
  KEYBOARD_VECTOR     = PIC_MASTER_VECTOR_RANGE_START + 1,

  MOUSE_VECTOR         = PIC_SLAVE_VECTOR_RANGE_START + 4,
  ATA_CHANNEL_1_VECTOR = PIC_SLAVE_VECTOR_RANGE_START + 6,

  FLOPPY_VECTOR = PIC_MASTER_VECTOR_RANGE_START + 6,
} IODeviceVector;

typedef enum {
  NON_AUTOMATIC_EOI = 0,
  AUTOMATIC_EOI     = 1,
} EOIType;

#pragma pack(push, 1)

typedef union {
  struct {
    uint16_t offset_low                       : 16;
    uint16_t segment_selector                 : 16;
    uint8_t reserved                          : 5;
    uint8_t fixed1                            : 3;
    GateDescriptorType type                   : 4;
    uint8_t fixed2                            : 1;
    PrivilegeLevel descriptor_privilege_level : 2;
    uint8_t present                           : 1;
    uint16_t offset_high                      : 16;
  } repr;

  uint64_t qword;
} GateDescriptor;

#pragma pack(pop)

typedef void (*InterruptHandler)(const Context* const);

extern GateDescriptor* idt;

void init_interrupts(void);
void set_interrupt_handler(uint8_t vector, GateDescriptorType type, PrivilegeLevel dpl, InterruptHandler handler);
void direct_set_interrupt_handler(uint8_t vector, GateDescriptorType type, PrivilegeLevel dpl, void (*handler)(void));
void init_pic(EOIType automatic_EOI);
void enable_io_devices(IODevice devices);
void disable_io_devices(IODevice devices);

#endif

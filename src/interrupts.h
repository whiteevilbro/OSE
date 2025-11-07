#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include "memmgnt.h"
#include "ports.h"

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
  TRAP_GATE = 0xF,
} GateDescriptorType;

typedef struct {
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

typedef enum {
  SYSTEM_TIMER = 0x1,
  KEYBOARD = 0x1 << 1,

  MOUSE = (0x1 << 4) | 0x100,
  ATA_CHANNEL_1 = (0x1 << 6) | 0x100,

  FLOPPY = 0x1 << 6,

  ALL_DEVICES = 0xffff,
} IODevice;

typedef enum {
  SYSTEM_TIMER_VECTOR = PIC_MASTER_VECTOR_RANGE_START,
  KEYBOARD_VECTOR = PIC_MASTER_VECTOR_RANGE_START + 1,

  MOUSE_VECTOR = PIC_SLAVE_VECTOR_RANGE_START + 4,
  ATA_CHANNEL_1_VECTOR = PIC_SLAVE_VECTOR_RANGE_START + 6,

  FLOPPY_VECTOR = PIC_MASTER_VECTOR_RANGE_START + 6,
} IODeviceVector;

typedef enum {
  NON_AUTOMATIC_EOI = 0,
  AUTOMATIC_EOI = 1,
} EOIType;

typedef void (*InterruptHandler)(const Context* const);

void init_interrupts(void);
void set_interrupt_handler(uint8_t vector, GateDescriptorType type, InterruptHandler handler);
void init_pic(EOIType automatic_EOI);
void enable_io_devices(IODevice devices);
void disable_io_devices(IODevice devices);

#endif
#ifndef UTILS_H_
#define UTILS_H_

#define noret void __attribute__((__noreturn__))

extern void* gdt;
extern void* tss;

extern noret halt(void);

extern noret jump_to_userspace(void (*func)(void), void* stack);

typedef enum {
  RING_0    = 0x0,
  RING_1    = 0x1,
  RING_2    = 0x2,
  RING_3    = 0x3,
  KERNEL_PL = RING_0,
  USER_PL   = RING_3,
} PrivilegeLevel;

typedef enum {
  KERNEL_CODE_SEGMENT = 0x8,
  KERNEL_DATA_SEGMENT = 0x10,
  APP_CODE_SEGMENT    = 0x18,
  APP_DATA_SEGMENT    = 0x20,
  TSS_SEGMENT         = 0x28,
} Segment;

#endif

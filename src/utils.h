#ifndef UTILS_H_
#define UTILS_H_

#define noret void __attribute__((__noreturn__))

extern noret halt(void);

typedef enum {
  RING_0    = 0x0,
  RING_1    = 0x1,
  RING_2    = 0x2,
  RING_3    = 0x3,
  KERNEL_PL = RING_0,
  USER_PL   = RING_3,
} PrivilegeLevel;

typedef enum {
  CODE_SEGMENT = 0x8,
  DATA_SEGMENT = 0x10,
} Segment;

#endif

#ifndef USER_CALLS_H_
#define USER_CALLS_H_

#define exit(status) \
  __asm__ __volatile__("int 0x31" ::"a"(status));

#endif

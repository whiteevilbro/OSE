#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <stdint.h>

#define cli() __asm__ __volatile__("cli" : : : "memory", "cc");
#define sti() __asm__ __volatile__("sti" : : : "memory", "cc");

#if __GNUC__ >= 15
  #define pushfd()                   \
    uint32_t __eflags;               \
    __asm__ __volatile__("pushf\n\t" \
                         "pop %0" : "=r"(__eflags) : : "memory", "redzone")
  #define popfd()                      \
    __asm__ __volatile__("push %0\n\t" \
                         "popf" : : "=r"(__eflags) : : "memory", "cc", "redzone")
#else
  #define pushfd()                   \
    uint32_t __eflags;               \
    __asm__ __volatile__("pushf\n\t" \
                         "pop %0" : "=r"(__eflags) : : "memory")
  #define popfd()                      \
    __asm__ __volatile__("push %0\n\t" \
                         "popf" : : "r"(__eflags) : "memory", "cc")
#endif

#endif
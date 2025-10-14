#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#define cli() __asm__ __volatile__("cli" : : : "memory");
#define sti() __asm__ __volatile__("sti" : : : "memory");

#endif
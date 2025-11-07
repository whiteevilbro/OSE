#ifndef PORTS_H_
#define PORTS_H_

#define outb(p, v) __asm__ __volatile__("out dx, al" : : "a"((v)), "d"((p)))
#define inb(p, v) __asm__ __volatile__("in al, dx" : "=a"((v)) : "d"((p)))

#endif
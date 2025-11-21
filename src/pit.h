#ifndef PIT_H_
#define PIT_H_

#include "ports.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  PIT_CHANNEL0 = 0x40,
  PIT_CHANNEL1 = 0x41,
  PIT_CHANNEL2 = 0x42,
} PITChannel;

typedef union {
  struct {
    bool bcd : 1;

    enum {
      INTERRUPT_ON_TERMINAL_COUNT     = 0x0,
      HARDWARE_RETRIGGERABLE_ONE_SHOT = 0x1,
      RATE_GENERATOR                  = 0x2,
      SQUARE_WAVE_MODE                = 0x3,
      SOFTWARE_TRIGGERED_STROBE       = 0x4,
      HARDWARE_TRIGGERED_STROBE       = 0x5,
    } mode : 3;

    enum {
      COUNTER_LATCH_COMMAND = 0x0,
      LSB_ONLY              = 0x1,
      MSB_ONLY              = 0x2,
      LSB_MSB               = 0x3,
    } read_write : 2;

    enum {
      COUNTER_0 = 0x0,
      COUNTER_1 = 0x1,
      COUNTER_2 = 0x2,
      READ_BACK = 0x3,
    } select_counter : 2;
  } repr;

  uint8_t byte;
} PITControlWord;

#define PIT_COMMAND_PORT 0x43

#define pit_command(cmd) outb((PIT_COMMAND_PORT), (cmd))
#define pit_send(ch, b) outb((ch), (b))

extern volatile uint32_t millis;

void init_timer(size_t frequency);
void init_timer_with_reload_value(uint32_t reload_value);

#endif

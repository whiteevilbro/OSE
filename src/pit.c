#include "pit.h"

#include "interrupts.h"

#define add_32fp32(h1, l1, h2, l2)      \
  __asm__ __volatile__("add %1, %3\n\t" \
                       "adc %0, %2" : "+rm"(h1), "+rm"(l1) : "ri"(h2), "ri"(l2) : "cc")

#define PIT_ACTUAL_FREQUENCY 3579545UL
#define PIT_INTERNAL_DIVIDER 3UL

static uint32_t millis_fractions = 0;
volatile uint32_t millis         = 0;

static uint32_t interrupt_millis           = 0;
static uint32_t interrupt_millis_fractions = 0;
static uint32_t interrupt_frequency        = 0;

static uint32_t PIT_reload_value_channel0 = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void timer_handler(const Context* const ctx) {
  add_32fp32(millis, millis_fractions, interrupt_millis, interrupt_millis_fractions);
}

#pragma GCC diagnostic pop

void init_timer(size_t frequency) {
  uint16_t reload_value;
  if (frequency >= (1 << 16)) {
    reload_value = 0;
  } else if (frequency < (PIT_ACTUAL_FREQUENCY / (1 << 16) / PIT_INTERNAL_DIVIDER)) {
    reload_value = (uint16_t) ~0x0;
  } else {
    reload_value = (uint16_t) ((PIT_ACTUAL_FREQUENCY / frequency) + (PIT_ACTUAL_FREQUENCY % frequency > frequency >> 1));
    reload_value = (uint16_t) ((reload_value / PIT_INTERNAL_DIVIDER) + (reload_value % PIT_INTERNAL_DIVIDER > PIT_INTERNAL_DIVIDER >> 1));
  }
  init_timer_with_reload_value(reload_value);
}

// reload_value more than 250 is strongly not recommended
void init_timer_with_reload_value(uint32_t reload_value) {
  if (reload_value == 1)
    reload_value++;
  else if (!reload_value) {
    reload_value |= 0x10000;
  }

  PIT_reload_value_channel0 = reload_value;

  size_t actual_frequency = (PIT_ACTUAL_FREQUENCY / reload_value) + (PIT_ACTUAL_FREQUENCY % reload_value > reload_value >> 1);
  actual_frequency        = (actual_frequency / PIT_INTERNAL_DIVIDER) + ((actual_frequency % PIT_INTERNAL_DIVIDER) > PIT_INTERNAL_DIVIDER >> 3);

  interrupt_frequency = actual_frequency;

  uint64_t mils              = (((PIT_INTERNAL_DIVIDER * 1000ULL * (1ULL << 42)) / PIT_ACTUAL_FREQUENCY + 1) * reload_value) >> 10;
  interrupt_millis           = (uint32_t) (mils >> 32);
  interrupt_millis_fractions = (uint32_t) mils;

  pushfd();
  cli();

  pit_command(((PITControlWord) {.repr = {.select_counter = COUNTER_0, .read_write = LSB_MSB, .mode = RATE_GENERATOR, .bcd = 0}}.byte));
  pit_send(PIT_CHANNEL0, (uint8_t) reload_value);
  pit_send(PIT_CHANNEL0, (uint8_t) (reload_value >> 8));

  popfd();

  set_interrupt_handler(SYSTEM_TIMER_VECTOR, INTERRUPT_GATE, &timer_handler);
}

#include "ps2.h"

#include "acpi.h"
#include "ports.h"

#include <stdbool.h>
#include <stdint.h>

#define MOTHERBOARD_8042_PRESENT_BOOT_FLAG 1 << 1

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

#define PS2_SELF_TEST_SUCCESS 0x55

typedef struct {
  bool OUTPUT_BUFFER_FULL : 1;
  bool INPUT_BUFFER_FULL  : 1;
  bool SYSTEM_FLAG        : 1;
  bool COMMAND_DATA       : 1;
  bool INHIBIT_SWITCH     : 1;
  bool TRANSMIT_TIMEOUT   : 1;
  bool RECIEVE_TIMEOUT    : 1;
  bool PARITY_ERROR       : 1;
} PS2Status;

typedef union {
  struct {
    bool channel0_input_buffer_full_interrupt : 1;
    bool channel1_input_buffer_full_interrupt : 1; // ps/2 compatible only
    bool system_flag                          : 1;
    bool inhibit_overrisde                    : 1; // at-compatible only
    bool disable_channel0_clock               : 1;
    bool disable_channel1_clock               : 1;
    bool enable_channel0_translation          : 1;
    uint8_t reserved                          : 1;
  } repr;

  uint8_t byte;
} PS2CommandByte;

typedef enum {
  READ_COMMAND_BYTE  = 0x20,
  WRITE_COMMAND_BYTE = 0x60,

  WRITE_TO_OUTPUT_PORT = 0x90, // 4 lsb are used

  GET_VERSION_NUMBER = 0xA1,

  GET_PASSWORD   = 0xA4,
  SET_PASSWORD   = 0xA5,
  CHECK_PASSWORD = 0xA6,

  DISABLE_CHANNEL1 = 0xA7,
  ENABLE_CHANNEL1  = 0xA8,
  CHANNEL1_TEST    = 0xA9,

  SELF_TEST = 0xAA,

  CHANNEL0_TEST    = 0xAB,
  DISABLE_CHANNEL0 = 0xAD,
  ENABLE_CHANNEL0  = 0xAE,

  GET_VERSION = 0xAF,

  READ_INPUT_PORT     = 0xC0,
  COPY_INPUT_PORT_LSN = 0xC1, // PS/2 only
  COPY_INPUT_PORT_MSN = 0xC2, // PS/2 only

  READ_OUTPUT_PORT      = 0xD0,
  WRITE_OUTPUT_PORT     = 0xD1,
  WRITE_CHANNEL0_BUFFER = 0xD2,
  WRITE_CHANNEL1_BUFFER = 0xD3,
  WRITE_CHANNEL1_DEVICE = 0xD4,

  READ_TEST_INPUTS = 0xE0,

  PULSE_OUTPUT_PORT = 0xF0, // 4 lsb are used
} PS2Command;

#pragma pack(push, 0)

typedef enum {
  OK                     = 0x0,
  TEST_FAILED_CLOCK_LOW  = 0x1,
  TEST_FAILED_CLOCK_HIGH = 0x2,
  TEST_FAILED_DATA_LOW   = 0x3,
  TEST_FAILED_DATA_HIGH  = 0x4,

  ABSENT        = 0x8,
  UNINITIALIZED = 0xf,
} PS2ChannelStatus;

static struct {
  PS2ChannelStatus channel0 : 4;
  PS2ChannelStatus channel1 : 4;
} ps2_status = {.channel0 = UNINITIALIZED, .channel1 = UNINITIALIZED};

#pragma pack(pop)

static bool ps2_controller_present(void) {
  if (!RSDP->revision || !FADT)
    return true;
  return FADT->iapc_boot_arch & MOTHERBOARD_8042_PRESENT_BOOT_FLAG;
}

static inline void ps2_send_command(PS2Command cmd) {
  PS2Status status;
  do {
    inb(PS2_STATUS_PORT, status);
  } while (status.INPUT_BUFFER_FULL);
  outb(PS2_COMMAND_PORT, cmd);
}

static inline void ps2_send_data(uint8_t data) {
  PS2Status status;
  do {
    inb(PS2_STATUS_PORT, status);
  } while (status.INPUT_BUFFER_FULL);
  outb(PS2_DATA_PORT, data);
}

static inline uint8_t ps2_read_data(void) {
  PS2Status status;
  do {
    inb(PS2_STATUS_PORT, status);
  } while (!status.OUTPUT_BUFFER_FULL);

  uint8_t data;
  inb(PS2_DATA_PORT, data);
  return data;
}

void init_ps2_controller(void) {
  if (!ps2_controller_present()) {
    ps2_status.channel0 = ABSENT;
    ps2_status.channel1 = ABSENT;
    return;
  }

  // Initializing

  ps2_send_command(DISABLE_CHANNEL0);
  ps2_send_command(DISABLE_CHANNEL1);

  uint8_t flush;
  inb(PS2_DATA_PORT, flush);

  PS2CommandByte cmd;

  //! DISABLE IRQ
  ps2_send_command(READ_COMMAND_BYTE);
  cmd.byte = ps2_read_data();

  cmd.repr.channel0_input_buffer_full_interrupt = false;
  cmd.repr.channel1_input_buffer_full_interrupt = false;
  cmd.repr.disable_channel0_clock               = false;
  cmd.repr.enable_channel0_translation          = false;

  ps2_send_command(WRITE_COMMAND_BYTE);
  ps2_send_data(cmd.byte);

  ps2_send_command(SELF_TEST);
  if (ps2_read_data() != PS2_SELF_TEST_SUCCESS)
    return;

  // Determine channel count

  ps2_send_command(ENABLE_CHANNEL1);
  ps2_send_command(READ_COMMAND_BYTE);
  cmd.byte = ps2_read_data();

  bool second_channel_present = !cmd.repr.disable_channel1_clock;

  if (second_channel_present) {
    ps2_send_command(DISABLE_CHANNEL1);
    ps2_send_command(READ_COMMAND_BYTE);
    cmd.byte = ps2_read_data();

    cmd.repr.channel1_input_buffer_full_interrupt = false;
    cmd.repr.disable_channel1_clock               = false;

    ps2_send_command(WRITE_COMMAND_BYTE);
    ps2_send_data(cmd.byte);
  } else {
    ps2_status.channel1 = ABSENT;
  }

  // Interface tests

  ps2_send_command(CHANNEL0_TEST);
  ps2_status.channel0 = ps2_read_data();

  if (second_channel_present) {
    ps2_send_data(CHANNEL1_TEST);
    ps2_status.channel1 = ps2_read_data();
  }
}

void ps2_reset_devices(void) {
  return;
}

#ifndef PS2_CONTROLLER_H_
#define PS2_CONTROLLER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

#pragma pack(push, 0)

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

typedef enum {
  CHANNEL_OK             = 0x0,
  TEST_FAILED_CLOCK_LOW  = 0x1,
  TEST_FAILED_CLOCK_HIGH = 0x2,
  TEST_FAILED_DATA_LOW   = 0x3,
  TEST_FAILED_DATA_HIGH  = 0x4,

  ABSENT        = 0x8,
  UNINITIALIZED = 0xf,
} PS2ChannelStatus;

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

typedef struct {
  PS2ChannelStatus channel0 : 4;
  PS2ChannelStatus channel1 : 4;
} PS2ControllerChannelStatus;

#pragma pack(pop)

extern PS2ControllerChannelStatus ps2_status;

void init_ps2_controller(void);
void ps2_send_command(PS2Command cmd);
void ps2_send_data(uint8_t data);

#endif

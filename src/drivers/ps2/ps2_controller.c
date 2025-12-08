#include "ps2_controller.h"

#include "acpi.h"
#include "interrupts.h"
#include "ports.h"

#include <stddef.h>
#include <stdint.h>
#include <threads.h>

#define MOTHERBOARD_8042_PRESENT_BOOT_FLAG 1 << 1

#define PS2_SELF_TEST_SUCCESS 0x55

PS2ControllerChannelStatus ps2_status = {.channel0 = UNINITIALIZED, .channel1 = UNINITIALIZED};

static bool ps2_controller_present(void) {
  if (!RSDP->revision || !FADT)
    return true;
  return FADT->iapc_boot_arch & MOTHERBOARD_8042_PRESENT_BOOT_FLAG;
}

void ps2_send_command(PS2Command cmd) {
  PS2Status status;
  do {
    inb(PS2_STATUS_PORT, status);
  } while (status.INPUT_BUFFER_FULL);
  outb(PS2_COMMAND_PORT, cmd);
}

void ps2_send_data(uint8_t data) {
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
  ps2_status.channel0 = (PS2ChannelStatus) ps2_read_data();

  if (second_channel_present) {
    ps2_send_command(CHANNEL1_TEST);
    ps2_status.channel1 = (PS2ChannelStatus) ps2_read_data();
  }

  disable_io_devices(KEYBOARD);
  disable_io_devices(MOUSE);

  ps2_send_command(READ_COMMAND_BYTE);
  cmd.byte = ps2_read_data();

  cmd.repr.channel0_input_buffer_full_interrupt = true;
  cmd.repr.channel1_input_buffer_full_interrupt = true;

  ps2_send_command(WRITE_COMMAND_BYTE);
  ps2_send_data(cmd.byte);
}

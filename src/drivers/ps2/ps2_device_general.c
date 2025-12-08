#include "ps2_device_general.h"

#include "logger.h"
#include "pit.h"
#include "ps2_controller.h"

#include <stddef.h>
#include <stdint.h>

#define TIMEOUT_MS 0
#define TIMEOUT_MSF 0x80000000

#define RESET_RETRY_COUNT 3
#define DETECT_RETRY_COUNT 3

#define COMMAND_QUEUE_SIZE 32

#define str(x) #x
#define xstr(x) str(x)

typedef enum {
  RESET        = 0xFF,
  DISABLE_SCAN = 0xF5,
  ENABLE_SCAN  = 0xF4,
  IDENTIFY     = 0xF2
} PS2ToDeviceCommand;

typedef enum {
  DEVICE_OK = 0x0,
  TIMED_OUT,
  BAD_ACKNOWLEDGEMENT,
  BAT_FAILED,
  COMMUNICATION_FAILED,
} PS2DeviceState;

static uint8_t channel0_command_buffer[COMMAND_QUEUE_SIZE];
static uint8_t channel1_command_buffer[COMMAND_QUEUE_SIZE];

PS2DeviceID devices[2] = {NOTHING, NOTHING};

CommandQueue command_queues[2] = {{channel0_command_buffer, 0, 0, COMMAND_QUEUE_SIZE},
                                  {channel1_command_buffer, 0, 0, COMMAND_QUEUE_SIZE}};

uint8_t ps2_device_send(uint8_t data) {
  uint32_t ms  = millis;
  uint32_t msf = millis_fractions;
  add_32fp32(ms, msf, TIMEOUT_MS, TIMEOUT_MSF);
  PS2Status status;
  while (ms > millis || (ms == millis && msf > millis_fractions)) {
    inb(PS2_STATUS_PORT, status);
    if (status.INPUT_BUFFER_FULL)
      continue;
    outb(PS2_DATA_PORT, data);
    return 0;
  }
  return 1;
}

uint16_t ps2_device_read(void) {
  uint32_t ms  = millis;
  uint32_t msf = millis_fractions;
  add_32fp32(ms, msf, TIMEOUT_MS, TIMEOUT_MSF);
  PS2Status status;
  uint8_t data;
  while (ms > millis || (ms == millis && msf > millis_fractions)) {
    inb(PS2_STATUS_PORT, status);
    if (!status.OUTPUT_BUFFER_FULL)
      continue;
    inb(PS2_DATA_PORT, data);
    return data;
  }
  return 0xffff;
}

static inline const char* get_error_reason(PS2DeviceState status) {
  switch (status) {
    case TIMED_OUT:
      return "Timed out";
    case BAD_ACKNOWLEDGEMENT:
      return "Bad acknowledgement";
    case BAT_FAILED:
      return "BAT failed";
    case COMMUNICATION_FAILED:
      return "Controller stuck busy";
    default:
      return "";
  }
}

static void ps2_reset_device(uint8_t channel) {
  PS2DeviceState status;
  uint16_t answer;

  if ((ps2_status.channel0 == CHANNEL_OK && channel == 0) || (ps2_status.channel1 == CHANNEL_OK && channel == 1)) {
    if (channel) {
      ps2_send_command(ENABLE_CHANNEL1);
    } else {
      ps2_send_command(ENABLE_CHANNEL0);
    }
    for (size_t tr = 1; tr <= RESET_RETRY_COUNT; tr++) {
      DEBUG("Trying to reset PS/2 channel %d. Try %d out of " xstr(RESET_RETRY_COUNT) "\n", channel, tr);
      if (channel) {
        ps2_send_command(WRITE_CHANNEL1_DEVICE);
      }
      if (ps2_device_send(RESET)) {
        DEBUG("PS/2 channel %d: Controller busy.\n", channel);
        status = COMMUNICATION_FAILED;
        continue;
      }


      if ((answer = ps2_device_read()) & 0xff00) {
        DEBUG("PS/2 channel %d: Device timed out.\n", channel);
        status = TIMED_OUT;
        continue;
      }
      if (answer != ACK) {
        DEBUG("PS/2 channel %d: non-ACK code recieved (%02X).\n", channel, answer);
        status = BAD_ACKNOWLEDGEMENT;
        continue;
      }

      if ((answer = ps2_device_read()) & 0xff00) {
        DEBUG("PS/2 channel %d: Device timed out.\n", channel);
        status = TIMED_OUT;
        continue;
      }
      if (answer != BAT) {
        DEBUG("PS/2 channel %d: BAT failed. Code received: (%02X).\n", channel, answer);
        status = BAT_FAILED;
        continue;
      }

      status = DEVICE_OK;
      break;
    }
    if (status) {
      ERROR("PS/2 channel %d: Retry count exceeded: %s. Aborting communication.\n", channel, get_error_reason(status));
    } else {
      INFO("PS/2 channel %d: device reset\n", channel);
    }
  }
}

void ps2_reset_devices(void) {
  for (uint8_t ch = 0; ch < 2; ch++) {
    ps2_reset_device(ch);
  }
}

static inline const char* get_device_name(PS2DeviceID id) {
  switch (id) {
    case AT_KEYBOARD:
      return "AT Keyboard";
    case PS2_MOUSE:
      return "Mouse";
    case PS2_MOUSE_SCROLL:
      return "Mouse with scroll wheel";
    case PS2_MOUSE_5_BUTTON:
      return "5-button mouse";
    case MF_KEYBOARD:
    case MF_KEYBOARD1:
      return "Multifuctional keyboard";
    case SHORT_KEYBOARD:
      return "Short keyboard";
    case NCD_N_97:
      return "NCD N-97 keyboard";
    case NCD_SUN:
      return "NCD Sun keyboard";
    case M122:
      return "122-key keyboard";
    case JAP_G:
      return "Japanese \"G\" keyboard";
    case JAP_P:
      return "Japanese \"P\" keyboard";
    case JAP_A:
      return "Japanese \"A\" keyboard";
    case NOTHING: // This shouldn't ever happen
    default:
      return "Nothing";
  }
}

static PS2DeviceID ps2_read_device_id(void) {
  uint16_t answer;
  PS2DeviceID device;
  if ((answer = ps2_device_read()) & 0xff00) {
    device = AT_KEYBOARD; // it doesn't send no ID
  } else {
    device = answer;
    if (!((answer = ps2_device_read()) & 0xff00)) {
      device |= (int64_t) (answer << 8);
    }
  }
  return device;
}

void ps2_detect_devices(void) {
  if (ps2_status.channel0 == CHANNEL_OK) {
    for (size_t ch = 1; ch <= DETECT_RETRY_COUNT; ch++) {
      ps2_device_send(DISABLE_SCAN);
      if (ps2_device_read() != ACK) {
        continue;
      }

      ps2_device_send(IDENTIFY);
      if (ps2_device_read() != ACK) {
        continue;
      }
      PS2DeviceID id = ps2_read_device_id();

      devices[0] = id;
      INFO("PS/2 channel 0: Device detected - %s\n", get_device_name(id));
      break;
    }
  }

  if (ps2_status.channel1 == CHANNEL_OK) {
    for (size_t ch = 1; ch <= DETECT_RETRY_COUNT; ch++) {
      ps2_send_command(WRITE_CHANNEL1_DEVICE);
      ps2_device_send(DISABLE_SCAN);
      if (ps2_device_read() != ACK) {
        continue;
      }

      ps2_send_command(WRITE_CHANNEL1_DEVICE);
      ps2_device_send(IDENTIFY);
      if (ps2_device_read() != ACK) {
        continue;
      }
      PS2DeviceID id = ps2_read_device_id();

      devices[1] = id;
      INFO("PS/2 channel 1: Device detected - %s\n", get_device_name(id));
      break;
    }
  }
}

// if it overflows - stop. Whatever you doing - stop it.
void ps2_device_enqueue_command(uint8_t channel, uint8_t cmd) {
  command_queues[channel].buffer[command_queues[channel].tail++] = cmd;
  command_queues[channel].tail %= command_queues[channel].size;
}

void ps2_device_send_topqueue(uint8_t channel) {
  if (command_queues[channel].head == command_queues[channel].tail)
    return;

  if (channel) {
    ps2_send_command(WRITE_CHANNEL1_DEVICE);
  }
  ps2_device_send(command_queues[channel].buffer[command_queues[channel].head]);
}

uint8_t ps2_device_queue_top(uint8_t channel) {
  return command_queues[channel].buffer[command_queues[channel].head];
}

void ps2_device_queue_pop(uint8_t channel) {
  command_queues[channel].head = (command_queues[channel].head + 1) % command_queues[channel].size;
}

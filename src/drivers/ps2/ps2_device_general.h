#ifndef PS2_DEVICES_H_
#define PS2_DEVICES_H_

#include <stddef.h>
#include <stdint.h>

#define COMMAND_RESEND_THRESHOLD 3

typedef enum {
  NOTHING = 0x20000,

  AT_KEYBOARD = 0x10000,

  PS2_MOUSE          = 0x0,
  PS2_MOUSE_SCROLL   = 0x3,
  PS2_MOUSE_5_BUTTON = 0x4,

  MF_KEYBOARD  = 0x83AB,
  MF_KEYBOARD1 = 0xC1AB,

  SHORT_KEYBOARD = 0x84AB,

  NCD_N_97 = 0x85AB,
  NCD_SUN  = 0xA1AC,
  M122     = 0x86AB,

  JAP_G = 0x90AB,
  JAP_P = 0x91AB,
  JAP_A = 0x92AB,
} PS2DeviceID;

typedef enum {
  ACK = 0xFA,
  RSD = 0xFE,
  ERR = 0xFC,
  BAT = 0xAA,
} PS2ToHostCommand;

typedef struct {
  uint8_t* buffer;
  size_t head;
  size_t tail;
  size_t size;
} CommandQueue;

extern PS2DeviceID devices[2];

// extern CommandQueue command_queues[2];

void ps2_reset_devices(void);
void ps2_detect_devices(void);

uint8_t ps2_device_send(uint8_t data);
uint16_t ps2_device_read(void);

void ps2_device_enqueue_command(uint8_t channel, uint8_t cmd);
void ps2_device_send_topqueue(uint8_t channel);
uint8_t ps2_device_queue_top(uint8_t channel);
void ps2_device_queue_pop(uint8_t channel);

#endif

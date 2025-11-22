#ifndef PS2_DEVICES_H_
#define PS2_DEVICES_H_

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

extern PS2DeviceID devices[2];

void ps2_reset_devices(void);
void ps2_detect_devices(void);

#endif

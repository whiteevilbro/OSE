#include "console.h" // IWYU pragma: keep
#include "interrupts.h"
#include "logger.h"
#include "ports.h"
#include "ps2_controller.h"
#include "ps2_device_general.h"
#include "ps2_keyboard_keys.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
  SET_LED                  = 0xED,
  ECHO                     = 0xEE,
  SET_SCAN_CODE_SET        = 0xF0,
  IDENTIFY                 = 0xF2,
  SET_TYPEMATIC_RATE_DELAY = 0xF3,

  ENABLE_SCAN  = 0xF4,
  DISABLE_SCAN = 0xF5,

  SET_DEFAULT = 0xF6,

  // scancode set 3 only start
  SET_ALL_KEYS_TYPEMATIC              = 0xF7,
  SET_ALL_KEYS_MAKE_RELEASE           = 0xF8,
  SET_ALL_KEYS_MAKE                   = 0xF9,
  SET_ALL_KEYS_TYPEMATIC_MAKE_RELEASE = 0xFA,

  SET_KEY_TYPEMATIC    = 0xFB,
  SET_KEY_MAKE_RELEASE = 0xFC,
  SET_KEY_MAKE         = 0xFD,
  // scancode set 3 only end

  RESEND = 0xFE,

  RESET = 0xFF,
} PS2KeyboardCommand;

typedef enum {
  DELAY_1_4_SECOND = 0,
  DELAY_1_2_SECOND = 1 << 5,
  DELAY_3_4_SECOND = 2 << 5,
  DELAY_1_SECOND   = 3 << 5,
} TypematicDelay;

typedef enum {
  RATE_30_0_HZ = 0x00,
  RATE_26_7_HZ = 0x01,
  RATE_24_0_HZ = 0x02,
  RATE_21_8_HZ = 0x03,
  RATE_20_7_HZ = 0x04,
  RATE_18_5_HZ = 0x05,
  RATE_17_1_HZ = 0x06,
  RATE_16_0_HZ = 0x07,
  RATE_15_0_HZ = 0x08,
  RATE_13_3_HZ = 0x09,
  RATE_12_0_HZ = 0x0A,
  RATE_10_9_HZ = 0x0B,
  RATE_10_0_HZ = 0x0C,
  RATE_9_2_HZ  = 0x0D,
  RATE_8_6_HZ  = 0x0E,
  RATE_8_0_HZ  = 0x0F,
  RATE_7_5_HZ  = 0x10,
  RATE_6_7_HZ  = 0x11,
  RATE_6_0_HZ  = 0x12,
  RATE_5_5_HZ  = 0x13,
  RATE_5_0_HZ  = 0x14,
  RATE_4_6_HZ  = 0x15,
  RATE_4_3_HZ  = 0x16,
  RATE_4_0_HZ  = 0x17,
  RATE_3_7_HZ  = 0x18,
  RATE_3_3_HZ  = 0x19,
  RATE_3_0_HZ  = 0x1A,
  RATE_2_7_HZ  = 0x1B,
  RATE_2_5_HZ  = 0x1C,
  RATE_2_3_HZ  = 0x1D,
  RATE_2_1_HZ  = 0x1E,
  RATE_2_0_HZ  = 0x1F,
} TypematicRepeatRate;

typedef enum {
  SCAN_CODE_GET   = 0,
  SCAN_CODE_SET_1 = 1,
  SCAN_CODE_SET_2 = 2,
  SCAN_CODE_SET_3 = 3,
} ScanCodeArgument;

typedef enum {
  LED_OFF     = 0x0,
  SCROLL_LOCK = 0x1,
  NUM_LOCK    = 0x2,
  CAPS_LOCK   = 0x4,
} LedStates;

void keyboard_handler(const Context* const ctx);
static void send_make_key(KeyCode code);
static void send_break_key(KeyCode code);

void init_ps2_keyboard(uint8_t channel) {
  if (channel) {
    ps2_send_command(ENABLE_CHANNEL1);
    set_interrupt_handler(MOUSE_VECTOR, INTERRUPT_GATE, keyboard_handler); //todo
  } else {
    ps2_send_command(ENABLE_CHANNEL0);
    set_interrupt_handler(KEYBOARD_VECTOR, INTERRUPT_GATE, keyboard_handler); //todo
  }

  ps2_device_enqueue_command(channel, SET_LED);
  ps2_device_enqueue_command(channel, LED_OFF);
  ps2_device_enqueue_command(channel, SET_SCAN_CODE_SET);
  ps2_device_enqueue_command(channel, SCAN_CODE_SET_2);
  ps2_device_enqueue_command(channel, SET_TYPEMATIC_RATE_DELAY);
  ps2_device_enqueue_command(channel, (uint8_t) DELAY_1_2_SECOND | (uint8_t) RATE_30_0_HZ);
  ps2_device_enqueue_command(channel, ENABLE_SCAN);

  if (channel) {
    enable_io_devices(MOUSE);
  } else {
    enable_io_devices(KEYBOARD);
  }

  ps2_device_send_topqueue(channel);
}

static void send_make_key(KeyCode code) {
  char c;
  switch (code) {
    case KEY_1:
    case KEY_KEYPAD_1:
      c = '1';
      break;
    case KEY_2:
    case KEY_KEYPAD_2:
      c = '2';
      break;
    case KEY_3:
    case KEY_KEYPAD_3:
      c = '3';
      break;
    case KEY_4:
    case KEY_KEYPAD_4:
      c = '4';
      break;
    case KEY_5:
    case KEY_KEYPAD_5:
      c = '5';
      break;
    case KEY_6:
    case KEY_KEYPAD_6:
      c = '6';
      break;
    case KEY_7:
    case KEY_KEYPAD_7:
      c = '7';
      break;
    case KEY_8:
    case KEY_KEYPAD_8:
      c = '8';
      break;
    case KEY_9:
    case KEY_KEYPAD_9:
      c = '9';
      break;
    case KEY_0:
      c = '0';
      break;
    case KEY_KEYPAD_PLUS:
      c = '+';
      break;
    case KEY_KEYPAD_MINUS:
    case KEY_MINUS:
      c = '-';
      break;
    case KEY_EQUALS:
      c = '=';
      break;
    case KEY_KEYPAD_SLASH:
      c = '/';
      break;
    case KEY_KEYPAD_ASTERISK:
      c = '*';
      break;
    case KEY_Q:
      c = 'q';
      break;
    case KEY_W:
      c = 'w';
      break;
    case KEY_E:
      c = 'e';
      break;
    case KEY_R:
      c = 'r';
      break;
    case KEY_T:
      c = 't';
      break;
    case KEY_Y:
      c = 'y';
      break;
    case KEY_U:
      c = 'u';
      break;
    case KEY_I:
      c = 'i';
      break;
    case KEY_O:
      c = 'o';
      break;
    case KEY_P:
      c = 'p';
      break;
    case KEY_OPEN_BRACKET:
      c = '[';
      break;
    case KEY_CLOSE_BRACKET:
      c = ']';
      break;
    case KEY_A:
      c = 'a';
      break;
    case KEY_S:
      c = 's';
      break;
    case KEY_D:
      c = 'd';
      break;
    case KEY_F:
      c = 'f';
      break;
    case KEY_G:
      c = 'g';
      break;
    case KEY_H:
      c = 'h';
      break;
    case KEY_J:
      c = 'j';
      break;
    case KEY_K:
      c = 'k';
      break;
    case KEY_L:
      c = 'l';
      break;
    case KEY_SEMICOLON:
      c = ';';
      break;
    case KEY_SINGLE_QUOTE:
      c = '\'';
      break;
    case KEY_SHARP:
      c = '#';
      break;
    case KEY_BACKSLASH:
      c = '\\';
      break;
    case KEY_Z:
      c = 'z';
      break;
    case KEY_X:
      c = 'x';
      break;
    case KEY_C:
      c = 'c';
      break;
    case KEY_V:
      c = 'v';
      break;
    case KEY_B:
      c = 'b';
      break;
    case KEY_N:
      c = 'n';
      break;
    case KEY_M:
      c = 'm';
      break;
    case KEY_COMMA:
      c = ',';
      break;
    case KEY_PERIOD:
      c = '.';
      break;
    case KEY_SLASH:
      c = '/';
      break;
    case KEY_SPACE:
      c = ' ';
      break;
    case KEY_BACKSPACE:
      c = '\b';
      break;

    case KEY_BACKTICK:
      c = '`';
      break;

    case KEY_ENTER:
    case KEY_KEYPAD_ENTER:
      c = '\n';
      break;

    case KEY_CAPS:
    case KEY_LSHIFT:
    case KEY_RSHIFT:
    case KEY_ESC:
    case KEY_APPS:
    case KEY_UP_ARROW:
    case KEY_LEFT_ARROW:
    case KEY_RIGHT_ARROW:
    case KEY_F1:
    case KEY_F2:
    case KEY_F3:
    case KEY_F4:
    case KEY_F5:
    case KEY_F6:
    case KEY_F7:
    case KEY_F8:
    case KEY_F9:
    case KEY_F10:
    case KEY_F11:
    case KEY_F12:
    case KEY_PRTSCR:
    case KEY_SCROLL_LOCK:
    case KEY_PAUSE:
    case KEY_INSERT:
    case KEY_HOME:
    case KEY_PAGE_UP:
    case KEY_NUM_LOCK:
    case KEY_TAB:
    case KEY_DELETE:
    case KEY_END:
    case KEY_PAGE_DOWN:
      return;

    case KEY_NONE:
    default:
      return;
  }
  cputc(c, stdout);
  cflush(stdout);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void send_break_key(KeyCode code) {
  return;
}

#pragma GCC diagnostic pop

// PrintScreen is a FUCKING DOGSHIT button
void keyboard_handler(const Context* const ctx) {
  uint8_t channel;
  if (ctx->vector == KEYBOARD_VECTOR) {
    channel = 0;
  } else {
    channel = 1;
  }

  uint8_t code;
  static uint8_t fail_count       = 0;
  static uint8_t bytes_to_receive = 0;
  static uint32_t fullcode        = 0;

  enum {
    BREAK_FLAG = 0x1,
    EXT_0_FLAG = 0x2,
    EXT_1_FLAG = 0x4,
  };

  static uint8_t flags;
  inb(PS2_DATA_PORT, code);
  KeyCode keycode;

  switch (code) {
    case ACK:
      fail_count = 0;
      ps2_device_queue_pop(channel);
      ps2_device_send_topqueue(channel);
      break;
    case RSD:
      fail_count++;
      if (fail_count == COMMAND_RESEND_THRESHOLD) {
        WARNING("PS/2 channel %d: Resend threshold hit. Command %#02X", channel, ps2_device_queue_top(channel));
        ps2_device_queue_pop(channel);
        break;
      }
      ps2_device_send_topqueue(channel);
      break;
    case 0xF0: // BREAK
      flags |= BREAK_FLAG;
      break;
    case 0xE0: // EXTEND ONE BYTE
      flags |= EXT_0_FLAG;
      break;
    case 0xE1: // EXTEND TWO BYTES
      flags |= EXT_1_FLAG;
      bytes_to_receive = 2;
      break;
    default:
      fullcode <<= 8;
      fullcode |= code;
      if (bytes_to_receive)
        bytes_to_receive--;
      if (bytes_to_receive) {
        return;
      }
      if (flags & EXT_1_FLAG) { // ext1
        keycode = fullcode == 0x1477 ? KEY_PAUSE : KEY_NONE;
      } else if (flags & EXT_0_FLAG) { // ext0
        switch (fullcode) {
          case 0x11:
            keycode = KEY_ALRGR;
            break;
          case 0x14:
            keycode = KEY_RCTRL;
            break;
          case 0x1F:
            keycode = KEY_LGUI;
            break;
          case 0x27:
            keycode = KEY_RGUI;
            break;
          case 0x2F:
            keycode = KEY_APPS;
            break;
          case 0x4A:
            keycode = KEY_KEYPAD_SLASH;
            break;
          case 0x5A:
            keycode = KEY_KEYPAD_ENTER;
            break;
          case 0x69:
            keycode = KEY_END;
            break;
          case 0x6B:
            keycode = KEY_LEFT_ARROW;
            break;
          case 0x6C:
            keycode = KEY_HOME;
            break;
          case 0x70:
            keycode = KEY_INSERT;
            break;
          case 0x71:
            keycode = KEY_DELETE;
            break;
          case 0x72:
            keycode = KEY_DOWN_ARROW;
            break;
          case 0x74:
            keycode = KEY_RIGHT_ARROW;
            break;
          case 0x75:
            keycode = KEY_UP_ARROW;
            break;
          case 0x7A:
            keycode = KEY_PAGE_DOWN;
            break;
          case 0x7D:
            keycode = KEY_PAGE_UP;
            break;

          case 0x12:
          case 0x7C:
            return;
          case 0x127C: // why are we here?
          case 0x7C12: // just to suffer?
            keycode = KEY_PRTSCR;
            break;
          default:
            keycode = KEY_NONE;
            break;
        }
      } else { // single byte
        if (code < BYTE_TO_CODE_SIZE) {
          keycode = byte_to_code[code];
        } else {
          keycode = KEY_NONE;
        }
      }

      if (flags & 0x01) {
        send_break_key(keycode);
      } else {
        // printf("%.2x\n", fullcode);
        send_make_key(keycode);
      }
      flags    = 0;
      fullcode = 0;
      break;
  }
}

#ifndef PS2_KEYBOARD_KEYS_H
#define PS2_KEYBOARD_KEYS_H

#define ROW_SHIFT 6
#define SHIFT_SHIFT (ROW_SHIFT + 3)

#define BYTE_TO_CODE_SIZE 128

typedef enum {
  KEY_NONE = -1,

  // ===== Row 0 =====
  KEY_ESC         = 0 << ROW_SHIFT | 0,
  // blank
  KEY_F1          = 0 << ROW_SHIFT | 2,
  KEY_F2          = 0 << ROW_SHIFT | 3,
  KEY_F3          = 0 << ROW_SHIFT | 4,
  KEY_F4          = 0 << ROW_SHIFT | 5,
  KEY_F5          = 0 << ROW_SHIFT | 6,
  KEY_F6          = 0 << ROW_SHIFT | 7,
  KEY_F7          = 0 << ROW_SHIFT | 8,
  KEY_F8          = 0 << ROW_SHIFT | 9,
  KEY_F9          = 0 << ROW_SHIFT | 10,
  KEY_F10         = 0 << ROW_SHIFT | 11,
  KEY_F11         = 0 << ROW_SHIFT | 12,
  KEY_F12         = 0 << ROW_SHIFT | 13,
  KEY_PRTSCR      = 0 << ROW_SHIFT | 14,
  KEY_SCROLL_LOCK = 0 << ROW_SHIFT | 15,
  KEY_PAUSE       = 0 << ROW_SHIFT | 16,

  // ===== Row 1 =====
  KEY_BACKTICK        = 1 << ROW_SHIFT | 0,
  KEY_1               = 1 << ROW_SHIFT | 1,
  KEY_2               = 1 << ROW_SHIFT | 2,
  KEY_3               = 1 << ROW_SHIFT | 3,
  KEY_4               = 1 << ROW_SHIFT | 4,
  KEY_5               = 1 << ROW_SHIFT | 5,
  KEY_6               = 1 << ROW_SHIFT | 6,
  KEY_7               = 1 << ROW_SHIFT | 7,
  KEY_8               = 1 << ROW_SHIFT | 8,
  KEY_9               = 1 << ROW_SHIFT | 9,
  KEY_0               = 1 << ROW_SHIFT | 10,
  KEY_MINUS           = 1 << ROW_SHIFT | 11,
  KEY_EQUALS          = 1 << ROW_SHIFT | 12,
  KEY_BACKSPACE       = 1 << ROW_SHIFT | 13,
  KEY_INSERT          = 1 << ROW_SHIFT | 14,
  KEY_HOME            = 1 << ROW_SHIFT | 15,
  KEY_PAGE_UP         = 1 << ROW_SHIFT | 16,
  KEY_NUM_LOCK        = 1 << ROW_SHIFT | 17,
  KEY_KEYPAD_SLASH    = 1 << ROW_SHIFT | 18,
  KEY_KEYPAD_ASTERISK = 1 << ROW_SHIFT | 19,
  KEY_KEYPAD_MINUS    = 1 << ROW_SHIFT | 20,

  // ===== Row 2 =====
  KEY_TAB           = 2 << ROW_SHIFT | 0,
  KEY_Q             = 2 << ROW_SHIFT | 1,
  KEY_W             = 2 << ROW_SHIFT | 2,
  KEY_E             = 2 << ROW_SHIFT | 3,
  KEY_R             = 2 << ROW_SHIFT | 4,
  KEY_T             = 2 << ROW_SHIFT | 5,
  KEY_Y             = 2 << ROW_SHIFT | 6,
  KEY_U             = 2 << ROW_SHIFT | 7,
  KEY_I             = 2 << ROW_SHIFT | 8,
  KEY_O             = 2 << ROW_SHIFT | 9,
  KEY_P             = 2 << ROW_SHIFT | 10,
  KEY_OPEN_BRACKET  = 2 << ROW_SHIFT | 11,
  KEY_CLOSE_BRACKET = 2 << ROW_SHIFT | 12,
  // blank
  KEY_DELETE        = 2 << ROW_SHIFT | 14,
  KEY_END           = 2 << ROW_SHIFT | 15,
  KEY_PAGE_DOWN     = 2 << ROW_SHIFT | 16,
  KEY_KEYPAD_7      = 2 << ROW_SHIFT | 17,
  KEY_KEYPAD_8      = 2 << ROW_SHIFT | 18,
  KEY_KEYPAD_9      = 2 << ROW_SHIFT | 19,
  // blank

  // ===== Row 3 =====
  KEY_CAPS         = 3 << ROW_SHIFT | 0,
  KEY_A            = 3 << ROW_SHIFT | 1,
  KEY_S            = 3 << ROW_SHIFT | 2,
  KEY_D            = 3 << ROW_SHIFT | 3,
  KEY_F            = 3 << ROW_SHIFT | 4,
  KEY_G            = 3 << ROW_SHIFT | 5,
  KEY_H            = 3 << ROW_SHIFT | 6,
  KEY_J            = 3 << ROW_SHIFT | 7,
  KEY_K            = 3 << ROW_SHIFT | 8,
  KEY_L            = 3 << ROW_SHIFT | 9,
  KEY_SEMICOLON    = 3 << ROW_SHIFT | 10,
  KEY_SINGLE_QUOTE = 3 << ROW_SHIFT | 11,
  KEY_SHARP        = 3 << ROW_SHIFT | 12,
  KEY_ENTER        = 3 << ROW_SHIFT | 13,
  // blank
  // blank
  // blank
  KEY_KEYPAD_4     = 3 << ROW_SHIFT | 17,
  KEY_KEYPAD_5     = 3 << ROW_SHIFT | 18,
  KEY_KEYPAD_6     = 3 << ROW_SHIFT | 19,
  KEY_KEYPAD_PLUS  = 3 << ROW_SHIFT | 20,

  // ===== Row 4 =====
  KEY_LSHIFT    = 4 << ROW_SHIFT | 0,
  KEY_BACKSLASH = 4 << ROW_SHIFT | 1,
  KEY_Z         = 4 << ROW_SHIFT | 2,
  KEY_X         = 4 << ROW_SHIFT | 3,
  KEY_C         = 4 << ROW_SHIFT | 4,
  KEY_V         = 4 << ROW_SHIFT | 5,
  KEY_B         = 4 << ROW_SHIFT | 6,
  KEY_N         = 4 << ROW_SHIFT | 7,
  KEY_M         = 4 << ROW_SHIFT | 8,
  KEY_COMMA     = 4 << ROW_SHIFT | 9,
  KEY_PERIOD    = 4 << ROW_SHIFT | 10,
  KEY_SLASH     = 4 << ROW_SHIFT | 11,
  // blank
  KEY_RSHIFT    = 4 << ROW_SHIFT | 13,
  // blank
  KEY_UP_ARROW  = 4 << ROW_SHIFT | 15,
  // blank
  KEY_KEYPAD_1  = 4 << ROW_SHIFT | 17,
  KEY_KEYPAD_2  = 4 << ROW_SHIFT | 18,
  KEY_KEYPAD_3  = 4 << ROW_SHIFT | 19,

  // ===== Row 5 =====
  KEY_LCTRL         = 5 << ROW_SHIFT | 0,
  KEY_LGUI          = 5 << ROW_SHIFT | 1,
  KEY_ALT           = 5 << ROW_SHIFT | 2,
  KEY_SPACE         = 5 << ROW_SHIFT | 3,
  // blank
  // blank
  // blank
  // blank
  // blank
  // blank
  KEY_ALRGR         = 5 << ROW_SHIFT | 10,
  KEY_RGUI          = 5 << ROW_SHIFT | 11,
  KEY_APPS          = 5 << ROW_SHIFT | 12,
  KEY_RCTRL         = 5 << ROW_SHIFT | 13,
  KEY_LEFT_ARROW    = 5 << ROW_SHIFT | 14,
  KEY_DOWN_ARROW    = 5 << ROW_SHIFT | 15,
  KEY_RIGHT_ARROW   = 5 << ROW_SHIFT | 16,
  KEY_KEYPAD_0      = 5 << ROW_SHIFT | 17,
  // blank
  KEY_KEYPAD_PERIOD = 5 << ROW_SHIFT | 19,
  KEY_KEYPAD_ENTER  = 5 << ROW_SHIFT | 20,

  // ========== SHIFT ==========
  // it's kinda unused

  // ===== Row 0 =====
  KEY_BREAK = 1 << SHIFT_SHIFT | 0 << ROW_SHIFT | 16,

  // ===== Row 1 ====
  KEY_VERTICAL_LINE     = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 0,
  KEY_EXCLAMATION_MARK  = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 1,
  KEY_DOUBLE_QUOTE      = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 2,
  KEY_POUND_SIGN        = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 3,
  KEY_DOLLAR_SIGN       = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 4,
  KEY_PERCENT           = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 5,
  KEY_CIRCUMFLEX        = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 6,
  KEY_AMPERSAND         = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 7,
  KEY_ASTERISK          = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 8,
  KEY_OPEN_PARENTHESIS  = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 9,
  KEY_CLOSE_PARENTHESIS = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 10,
  KEY_UNDERSCORE        = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 11,
  KEY_PLUS              = 1 << SHIFT_SHIFT | 1 << ROW_SHIFT | 12,

  // ===== Row 2 =====
  KEY_OPEN_CURVE  = 1 << SHIFT_SHIFT | 2 << ROW_SHIFT | 11,
  KEY_CLOSE_CURVE = 1 << SHIFT_SHIFT | 2 << ROW_SHIFT | 12,

  // ===== Row 3 =====
  KEY_COLON = 1 << SHIFT_SHIFT | 3 << ROW_SHIFT | 10,
  KEY_AT    = 1 << SHIFT_SHIFT | 3 << ROW_SHIFT | 11,
  KEY_TILDE = 1 << SHIFT_SHIFT | 3 << ROW_SHIFT | 12,

  // ===== Row 4 =====
  KEY_BROKEN_BAR    = 1 << SHIFT_SHIFT | 4 << ROW_SHIFT | 1,
  KEY_LESS_THAN     = 1 << SHIFT_SHIFT | 4 << ROW_SHIFT | 9,
  KEY_GREATER_THAN  = 1 << SHIFT_SHIFT | 4 << ROW_SHIFT | 10,
  KEY_QUESTION_MARK = 1 << SHIFT_SHIFT | 4 << ROW_SHIFT | 11,
} KeyCode;

extern const KeyCode byte_to_code[BYTE_TO_CODE_SIZE];

#endif

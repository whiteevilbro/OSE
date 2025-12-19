#include "console.h"

#include "memmgnt.h"
#include "vga.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ESC '\x1b'

#define isdigit(c) ('0' <= c && c <= '9')

Console fullscreen = {.default_format = {.background = BLACK, .foreground = WHITE}};
Console* stdout;

void init_consoles(void) {
  fullscreen.buffer      = malloc_immortal(STDOUT_BUFFER_SIZE, 8);
  fullscreen.buffer_size = STDOUT_BUFFER_SIZE;
  fullscreen.buffer_pos  = 0;

  vga_init_printer(24, 80);
  fullscreen.position = (Point){.x = 0, .y = 0};
  fullscreen.size     = (Point){.x = 80, .y = 24};

  fullscreen.cursor = (Point){.x = 0, .y = 0};
  fullscreen.format = fullscreen.default_format;

  stdout = &fullscreen;
  clear_console(stdout);
}

void clear_console(Console* console) {
  vga_fill(console->position, console->size,
           (VGAChar){.repr = {.character = ' ',
                              .fgcolor   = console->format.foreground,
                              .bgcolor   = console->format.background}});
  console->cursor = (Point){0, 0};
  vga_flush();
}

static char* bound_string_putchar(char* string, char* const end, char c) {
  if (string < end)
    *string = c;
  return ++string;
}

static size_t bound_string_length(const char* string, size_t count) {
  size_t length = 0;
  while (*string && count) {
    string++;
    count--;
    length++;
  }
  return length;
}

static const char* parse_int(const char* string, int* number_ptr) {
  int result = 0;
  char c;
  while ('0' <= (c = *string) && c <= '9') {
    result *= 10;
    result += c - '0';
    string++;
  }
  *number_ptr = result;
  return string;
}

enum {
  LEFT_FLAG   = 0x1,
  PLUS_FLAG   = 0x2,
  SPACE_FLAG  = 0x4,
  ALT_FLAG    = 0x8,
  ZERO_FLAG   = 0x10,
  UPPER_FLAG  = 0x20,
  SIGNED_FLAG = 0x40,
};

static char upper_digits[] = "0123456789ABCDEF";
static char lower_digits[] = "0123456789abcdef";

static char* bound_string_format_integer(char* string, char* const end, uint32_t number, uint8_t base, int width, int prescision, uint32_t flags) {
  if (prescision >= 0)
    flags &= ~(uint32_t) ZERO_FLAG;

  // sign processing
  char sign = 0;
  if (flags & SIGNED_FLAG) {
    if ((int32_t) number < 0) {
      number = (uint32_t) - (int32_t) number;
      sign   = '-';
      width--;
    } else if (flags & PLUS_FLAG) {
      sign = '+';
      width--;
    } else if (flags & SPACE_FLAG) {
      sign = ' ';
      width--;
    }
  }

  // number processing
  char reversed_buffer[22 + 1];
  char* pos = reversed_buffer;

  char* digits = flags & UPPER_FLAG ? upper_digits : lower_digits;
  if (number == 0) {
    *pos++ = '0';
  } else {
    while (number) {
      *pos++ = digits[number % base];
      number /= base;
    }
  }

  int num_length = (int) (pos - reversed_buffer);
  if (num_length > prescision) {
    prescision = num_length;
  }
  width -= prescision;

  bool prefix_needed = (flags & ALT_FLAG) && (base != 10);
  if (prefix_needed) {
    width--;
    if (base == 16) {
      width--;
    }
  }

  char padding = (flags & ZERO_FLAG) ? '0' : ' ';

  // space padding
  if (!(flags & (ZERO_FLAG | LEFT_FLAG))) {
    while (width-- > 0) {
      string = bound_string_putchar(string, end, ' ');
    }
  }
  // sign
  if (sign) {
    string = bound_string_putchar(string, end, sign);
  }
  // prefix
  if (prefix_needed) {
    string = bound_string_putchar(string, end, '0');
    if (base == 16) {
      string = bound_string_putchar(string, end, flags & UPPER_FLAG ? 'X' : 'x');
    }
  }
  // width zero padding
  if (!(flags & LEFT_FLAG)) { // implicitly && (flags & ZERO_FLAG)
    while (width-- > 0) {
      string = bound_string_putchar(string, end, padding); //todo padding == '0'
    }
  }
  // prescision zero padding
  while (prescision-- > num_length) {
    string = bound_string_putchar(string, end, '0');
  }
  // number
  while (num_length-- > 0) {
    string = bound_string_putchar(string, end, reversed_buffer[num_length]);
  }
  // implicitly flags & LEFT_FLAG
  // right side space padding
  while (width-- > 0) {
    string = bound_string_putchar(string, end, ' ');
  }
  return string;
}

int vsnprintf(char* s, size_t n, const char* format, va_list arg) {
  char c;

  char* pos = s;
  char* end = s + n; // technically, there could be an overflow, but wtf are you doing if it has happend

  uint8_t base;
  uint8_t flags; // see flag enum higher
  int width;
  int prescision;
  char qualifier;

  const char* str;
  const void* p;
  uint32_t number;
  size_t length;

  for (; (c = *format); format++) {
    if (c != '%') {
      pos = bound_string_putchar(pos, end, c);
      continue;
    }

    flags = 0;
    do {
      c = *++format;
      if (*format == '#') {
        flags |= ALT_FLAG;
      } else if (*format == '0') {
        flags |= ZERO_FLAG;
      } else if (*format == '+') {
        flags |= PLUS_FLAG;
      } else if (*format == ' ') {
        flags |= SPACE_FLAG;
      } else if (*format == '-') {
        flags |= LEFT_FLAG;
      } else {
        break;
      }
    } while (true);


    if (flags & PLUS_FLAG)
      flags &= (uint8_t) ~(uint8_t) SPACE_FLAG;
    if (flags & LEFT_FLAG)
      flags &= (uint8_t) ~(uint8_t) ZERO_FLAG;


    width = -1;
    if (c == '*') {
      format++;
      width = va_arg(arg, int);
      if (width < 0) {
        width = -width;
        flags |= LEFT_FLAG;
      }
    } else if (isdigit(c)) {
      format = parse_int(format, &width);
    }

    prescision = -1;
    if (*format == '.') {
      c = *++format;
      if (isdigit(c)) {
        format = parse_int(format, &prescision);
      } else if (c == '*') {
        format++;
        prescision = va_arg(arg, int);
      }
      if (prescision < 0)
        prescision = 0;
    }

    qualifier = -1;
    if ((c = *format) == 'h' || c == 'l' || c == 'L') {
      qualifier = c;
      format++;
    }

    c = *format;
    switch (c) {
      case '%':
        pos = bound_string_putchar(pos, end, '%');
        continue;

      case 'c': //todo l qualifier
        if (!(flags & LEFT_FLAG)) {
          while (--width > 0) // width - 1 spaces
            pos = bound_string_putchar(pos, end, ' ');
        }
        pos = bound_string_putchar(pos, end, (char) va_arg(arg, int));
        while (--width > 0)
          pos = bound_string_putchar(pos, end, ' ');
        continue;


      case 's': //todo l qualifier
        str    = va_arg(arg, const char*);
        length = bound_string_length(str, (size_t) prescision);
        if (!(flags & LEFT_FLAG)) {
          while (width-- > (int) length)
            pos = bound_string_putchar(pos, end, ' ');
        }
        for (size_t i = 0; i < length; i++) {
          pos = bound_string_putchar(pos, end, *str++);
        }
        while (width-- > (int) length)
          pos = bound_string_putchar(pos, end, ' ');
        continue;

      case 'o':
        base = 8;
        break;

      case 'i':
      case 'd':
        flags |= SIGNED_FLAG;
        // FALLTHROUGH
      case 'u':
        base = 10;
        break;

      case 'X':
        flags |= UPPER_FLAG;
        // FALLTHROUGH
      case 'x':
        base = 16;
        break;

      case 'p':
        if (width == -1) {
          width = 2 * sizeof(void*);
          flags |= ZERO_FLAG;
        }
        p   = va_arg(arg, void*);
        pos = bound_string_format_integer(pos, end, (uint32_t) p, 16, width, prescision, flags);
        continue;

      case 'n': //todo implement
        continue;

      default: //todo implement
        continue;
    }
    // number printing
    if (qualifier == 'l') {
      if (flags & SIGNED_FLAG)
        number = (uint32_t) va_arg(arg, long);
      else
        number = (uint32_t) va_arg(arg, unsigned long);
    } else if (qualifier == 'h') {
      if (flags & SIGNED_FLAG)
        number = (uint32_t) (short) va_arg(arg, int);
      else
        number = (uint32_t) (unsigned short) va_arg(arg, unsigned int);
    } else {
      if (flags & SIGNED_FLAG)
        number = (uint32_t) va_arg(arg, int);
      else
        number = (uint32_t) va_arg(arg, unsigned int);
    }
    pos = bound_string_format_integer(pos, end, number, base, width, prescision, flags);
  }
  if (n > 0) {
    if (pos < end) {
      *pos = '\0';
    } else {
      end[-1] = '\0';
    }
  }
  return (int) (pos - s);
}

int cprintf(Console* console, const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  int ret = vcprintf(console, format, arg);
  va_end(arg);
  return ret;
}

int printf(const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  int ret = vcprintf(stdout, format, arg);
  va_end(arg);
  return ret;
}

int sprintf(char* s, const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  int ret = vsprintf(s, format, arg);
  va_end(arg);
  return ret;
}

int snprintf(char* s, size_t n, const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  int ret = vsnprintf(s, n, format, arg);
  va_end(arg);
  return ret;
}

int vcprintf(Console* console, const char* format, va_list arg) {
  size_t size = console->buffer_size - console->buffer_pos;
  int ret     = vsnprintf(console->buffer + console->buffer_pos, size, format, arg);
  if (ret >= 0) {
    if ((size_t) ret < size)
      console->buffer_pos += (size_t) ret;
    else
      console->buffer_pos += size;
    cflush(console);
  }
  return ret;
}

int vprintf(const char* format, va_list arg) {
  return vcprintf(stdout, format, arg);
}

int vsprintf(char* s, const char* format, va_list arg) {
  return vsnprintf(s, SIZE_MAX, format, arg);
}

int cputc(int character, Console* console) {
  if (console->buffer_pos >= console->buffer_size) {
    cflush(console);
  }
  console->buffer[console->buffer_pos++] = (char) character;
  return (char) character;
}

int cputs(const char* s, Console* const console) {
  char c;
  while ((c = *s++)) {
    cputc(c, console);
  }
  return 0;
}

int putchar(int character) {
  return putc(character, stdout);
}

int puts(const char* s) {
  int ret = cputs(s, stdout);
  putchar('\n');
  return ret;
}

// ========================== WRITING TO VGA ==========================

typedef enum {
  ANSI_FOREGROUND_BLACK   = 30,
  ANSI_FOREGROUND_RED     = 31,
  ANSI_FOREGROUND_GREEN   = 32,
  ANSI_FOREGROUND_YELLOW  = 33,
  ANSI_FOREGROUND_BLUE    = 34,
  ANSI_FOREGROUND_MAGENTA = 35,
  ANSI_FOREGROUND_CYAN    = 36,
  ANSI_FOREGROUND_WHITE   = 37,

  ANSI_FOREGROUND_DEFAULT = 39,

  ANSI_FOREGROUND_BRIGHT_BLACK   = 90,
  ANSI_FOREGROUND_BRIGHT_RED     = 91,
  ANSI_FOREGROUND_BRIGHT_GREEN   = 92,
  ANSI_FOREGROUND_BRIGHT_YELLOW  = 93,
  ANSI_FOREGROUND_BRIGHT_BLUE    = 94,
  ANSI_FOREGROUND_BRIGHT_MAGENTA = 95,
  ANSI_FOREGROUND_BRIGHT_CYAN    = 96,
  ANSI_FOREGROUND_BRIGHT_WHITE   = 97,


  ANSI_BACKGROUND_BLACK   = 40,
  ANSI_BACKGROUND_RED     = 41,
  ANSI_BACKGROUND_GREEN   = 42,
  ANSI_BACKGROUND_YELLOW  = 43,
  ANSI_BACKGROUND_BLUE    = 44,
  ANSI_BACKGROUND_MAGENTA = 45,
  ANSI_BACKGROUND_CYAN    = 46,
  ANSI_BACKGROUND_WHITE   = 47,

  ANSI_BACKGROUND_DEFAULT = 49,

  ANSI_BACKGROUND_BRIGHT_BLACK   = 100,
  ANSI_BACKGROUND_BRIGHT_RED     = 101,
  ANSI_BACKGROUND_BRIGHT_GREEN   = 102,
  ANSI_BACKGROUND_BRIGHT_YELLOW  = 103,
  ANSI_BACKGROUND_BRIGHT_BLUE    = 104,
  ANSI_BACKGROUND_BRIGHT_MAGENTA = 105,
  ANSI_BACKGROUND_BRIGHT_CYAN    = 106,
  ANSI_BACKGROUND_BRIGHT_WHITE   = 107,
} ANSI_4_BIT_COLOR;

static void process_graphic_escape_sequence(int code, Console* console) {
  // color
  if (!code) {
    console->format = console->default_format;
  }
  if ((ANSI_FOREGROUND_BLACK <= code && code <= ANSI_FOREGROUND_WHITE) || (ANSI_FOREGROUND_BRIGHT_BLACK <= code && code <= ANSI_FOREGROUND_BRIGHT_WHITE) ||
      (ANSI_BACKGROUND_BLACK <= code && code <= ANSI_BACKGROUND_WHITE) || (ANSI_BACKGROUND_BRIGHT_BLACK <= code && code <= ANSI_BACKGROUND_BRIGHT_WHITE)) {
    switch ((ANSI_4_BIT_COLOR) code) {
      case ANSI_FOREGROUND_DEFAULT:
        console->format.foreground = console->default_format.foreground;
        return;
      case ANSI_BACKGROUND_DEFAULT:
        console->format.background = console->default_format.background;
        return;

      case ANSI_FOREGROUND_BLACK:
        console->format.foreground = BLACK;
        return;
      case ANSI_FOREGROUND_RED:
        console->format.foreground = RED;
        return;
      case ANSI_FOREGROUND_GREEN:
        console->format.foreground = GREEN;
        return;
      case ANSI_FOREGROUND_YELLOW:
        console->format.foreground = BROWN;
        return;
      case ANSI_FOREGROUND_BLUE:
        console->format.foreground = BLUE;
        return;
      case ANSI_FOREGROUND_MAGENTA:
        console->format.foreground = MAGENTA;
        return;
      case ANSI_FOREGROUND_CYAN:
        console->format.foreground = CYAN;
        return;
      case ANSI_FOREGROUND_WHITE:
        console->format.foreground = LIGHT_GRAY;
        return;
      case ANSI_FOREGROUND_BRIGHT_BLACK:
        console->format.foreground = DARK_GRAY;
        return;
      case ANSI_FOREGROUND_BRIGHT_RED:
        console->format.foreground = LIGHT_RED;
        return;
      case ANSI_FOREGROUND_BRIGHT_GREEN:
        console->format.foreground = LIGHT_GREEN;
        return;
      case ANSI_FOREGROUND_BRIGHT_YELLOW:
        console->format.foreground = YELLOW;
        return;
      case ANSI_FOREGROUND_BRIGHT_BLUE:
        console->format.foreground = LIGHT_BLUE;
        return;
      case ANSI_FOREGROUND_BRIGHT_MAGENTA:
        console->format.foreground = LIGHT_MAGENTA;
        return;
      case ANSI_FOREGROUND_BRIGHT_CYAN:
        console->format.foreground = LIGHT_CYAN;
        return;
      case ANSI_FOREGROUND_BRIGHT_WHITE:
        console->format.foreground = WHITE;
        return;

      case ANSI_BACKGROUND_BLACK:
        console->format.background = BLACK;
        return;
      case ANSI_BACKGROUND_RED:
        console->format.background = RED;
        return;
      case ANSI_BACKGROUND_GREEN:
        console->format.background = GREEN;
        return;
      case ANSI_BACKGROUND_YELLOW:
        console->format.background = BROWN;
        return;
      case ANSI_BACKGROUND_BLUE:
        console->format.background = BLUE;
        return;
      case ANSI_BACKGROUND_MAGENTA:
        console->format.background = MAGENTA;
        return;
      case ANSI_BACKGROUND_CYAN:
        console->format.background = CYAN;
        return;
      case ANSI_BACKGROUND_WHITE:
        console->format.background = LIGHT_GRAY;
        return;
      case ANSI_BACKGROUND_BRIGHT_BLACK:
        console->format.background = DARK_GRAY;
        return;
      case ANSI_BACKGROUND_BRIGHT_RED:
        console->format.background = LIGHT_RED;
        return;
      case ANSI_BACKGROUND_BRIGHT_GREEN:
        console->format.background = LIGHT_GREEN;
        return;
      case ANSI_BACKGROUND_BRIGHT_YELLOW:
        console->format.background = YELLOW;
        return;
      case ANSI_BACKGROUND_BRIGHT_BLUE:
        console->format.background = LIGHT_BLUE;
        return;
      case ANSI_BACKGROUND_BRIGHT_MAGENTA:
        console->format.background = LIGHT_MAGENTA;
        return;
      case ANSI_BACKGROUND_BRIGHT_CYAN:
        console->format.background = LIGHT_CYAN;
        return;
      case ANSI_BACKGROUND_BRIGHT_WHITE:
        console->format.background = WHITE;
        return;
    }
  }
}

// ONLY CSI<n>m at the moment
static const char* process_escape_sequence(const char* string, Console* console) {
  const char* pos = string;

  if (*pos++ != '[')
    return string;
  char c;
  if ('0' <= (c = *pos) && c <= '9') {
    int n;
    pos = parse_int(pos, &n);
    switch (*pos++) {
      case 'm':
        process_graphic_escape_sequence(n, console);
      default:
        break;
    }
  }
  return pos;
}

static inline void inplace_cwrite(int character, Console* console) {
  vga_print_char((VGAChar){.repr = {
                               .character = (char) character,
                               .fgcolor   = console->format.foreground,
                               .bgcolor   = console->format.background}},
                 console->position.x + console->cursor.x, console->position.y + console->cursor.y);
}

static void cwrite(int character, Console* console) {
  inplace_cwrite(character, console);
  if (++console->cursor.x / console->size.x) {
    console->cursor.y++;
    console->cursor.x = 0;
  }
}

static inline char cget_char(Console* console) {
  return vga_get_char(console->position.x + console->cursor.x, console->position.y + console->cursor.y).repr.character;
}

static void cscroll(Console* console) {
  vga_copy(console->position,
           (Point){.x = console->position.x, .y = console->position.y + 1},
           (Point){.x = console->size.x, .y = console->size.y - 1});
  vga_fill((Point){.x = console->position.x, .y = console->position.y + console->size.y - 1},
           (Point){.x = console->size.x, .y = 1},
           (VGAChar){.repr = {.character = ' ', .fgcolor = console->format.foreground, .bgcolor = console->format.background}});
  if (console->cursor.y)
    console->cursor.y--;
}

int cflush(Console* console) {
  if (!console->buffer_pos)
    return 0;
  const char* pos = console->buffer;
  const char* end = pos + console->buffer_pos;
  char c;

  size_t bottom_bound = console->position.y + console->size.y;

  while (pos < end) {
    if (console->cursor.y >= bottom_bound)
      cscroll(console);

    switch ((c = *pos)) {
      case '\n':
        console->cursor.y++;
        if (console->cursor.y >= bottom_bound)
          cscroll(console);
      case '\r':
        console->cursor.x = 0;
        break;
      case '\b':
        if (console->cursor.x) {
          console->cursor.x--;
        } else {
          console->cursor.x = console->size.x - 1;
          if (console->cursor.y)
            console->cursor.y--;
          while (cget_char(console) == ' ' && console->cursor.x) {
            console->cursor.x--;
            c = 0;
          }
          if (!c)
            console->cursor.x++;
        }
        inplace_cwrite(' ', console);
        break;
      case ESC: //todo
        pos = process_escape_sequence(++pos, console);
        continue;
      default:
        cwrite(c, console);
        break;
    }
    pos++;
  }
  console->buffer_pos = 0;
  vga_flush();
  return 0;
}

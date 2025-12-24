#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// =============== IO ===============

extern void print_char(int character);

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

#define isdigit(c) ('0' <= c && c <= '9')

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

static size_t bound_string_length(const char* string, size_t count) {
  size_t length = 0;
  while (*string && count) {
    string++;
    count--;
    length++;
  }
  return length;
}

static void format_integer(uint32_t number, uint8_t base, int width, int prescision, uint32_t flags) {
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
      print_char(' ');
    }
  }
  // sign
  if (sign) {
    print_char(sign);
  }
  // prefix
  if (prefix_needed) {
    print_char('0');
    if (base == 16) {
      print_char(flags & UPPER_FLAG ? 'X' : 'x');
    }
  }
  // width zero padding
  if (!(flags & LEFT_FLAG)) { // implicitly && (flags & ZERO_FLAG)
    while (width-- > 0) {
      print_char(padding);
    }
  }
  // prescision zero padding
  while (prescision-- > num_length) {
    print_char('0');
  }
  // number
  while (num_length-- > 0) {
    print_char(reversed_buffer[num_length]);
  }
  // implicitly flags & LEFT_FLAG
  // right side space padding
  while (width-- > 0) {
    print_char(' ');
  }
}

void vprintf(const char* format, va_list arg) {
  char c;

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
      print_char(c);
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
        print_char('%');
        continue;

      case 'c': //todo l qualifier
        if (!(flags & LEFT_FLAG)) {
          while (--width > 0) // width - 1 spaces
            print_char(' ');
        }
        print_char(va_arg(arg, int));
        while (--width > 0)
          print_char(' ');
        continue;


      case 's': //todo l qualifier
        str    = va_arg(arg, const char*);
        length = bound_string_length(str, (size_t) prescision);
        if (!(flags & LEFT_FLAG)) {
          while (width-- > (int) length)
            print_char(' ');
        }
        for (size_t i = 0; i < length; i++) {
          print_char(*str++);
        }
        while (width-- > (int) length)
          print_char(' ');
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
        p = va_arg(arg, void*);
        format_integer((uint32_t) p, 16, width, prescision, flags);
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
    format_integer(number, base, width, prescision, flags);
  }
}

void printf(const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  vprintf(format, arg);
  va_end(arg);
}

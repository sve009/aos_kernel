#include "kprint.h"

#include "term.h"

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Term-write
*/

term_write_t term_write = NULL;

// Set it
void set_term_write(term_write_t fn) {
  term_write = fn;
}

/*
   * Printing functions
 */

// Print one char
void kprint_c(char c) {
    term_putchar(c);
}

// Print string
void kprint_s(const char* str) {
    // Iterate through string and print each char
    while (*str != '\0') {
        kprint_c(*str);
        str += 1;
    }
}

// Print uint64
void kprint_d(uint64_t value) {
    // Store digit values
    char buffer[21] = { '\0' };
    int curr = 19;

    // Base case:
    if (value == 0) {
      kprint_c('0');
      return;
    }

    // Iterate through each digit.
    // Based off of:
    // https://stackoverflow.com/questions/3389264/how-to-get-the-separate-digits-of-an-int-number
    while (value > 0) {
        int d = value % 10;
        value /= 10;
        char c = d + 48; // 48 = 0 in ascii
        buffer[curr--] = c;
    }

    // Strip leading white spaces
    char* str = buffer;
    while (*str == '\0') {
      str++;
    }

    // Print
    kprint_s(str);
}

// Print hex
void kprint_x(uint64_t value) {
    // Store values
    char buffer[21] = { '\0' };
    int curr = 19;

    // Base case
    if (value == 0) {
      kprint_c('0');
      return;
    }

    // Iterate through each digit:
    while (value > 0) {
        int d = value % 16;

        char c;

        // Convert to char
        if (d == 10) {
            c = 'A';
        } else if (d == 11) {
            c = 'B';
        } else if (d == 12) {
            c = 'C';
        } else if (d == 13) {
            c = 'D';
        } else if (d == 14) {
            c = 'E';
        } else if (d == 15) {
            c = 'F';
        } else {
            c = d + 48; // Again add for ascii
        }

        // Divide d
        value /= 16;

        // Add to buffer
        buffer[curr--] = c;
    }

    // Strip whitespace
    char* str = buffer;
    while (*str == '\0') {
        str++;
    }

    // Print
    kprint_s(str);
}

// Print limited hex
void kprint_xd(uint16_t v) {
  kprint_x(v);
}

// Print pointer
void kprint_p(void* ptr) {
    kprint_s("0x");
    kprint_x((uint64_t)ptr);
}

// Our printf
void kprintf(const char* format, ...) {
  // Keep track of variable arguments
  va_list args;

  // Initialize
  va_start(args, format);  

  bool flag = false;
  
  // Iterate through string
  while (*format != '\0') {
    // Default case
    if (*format != '%' && !flag) {
      kprint_c(*format);
    // % case, activates format flag
    } else if (*format == '%' && !flag) {
      flag = true;
    // %% case
    } else if (*format == '%') {
      kprint_c('%');
      flag = false; 
    } else {
      // Print appropriate arg after % flag
      switch (*format) {
        case 's': kprint_s(va_arg(args, char*)); break;
        case 'd': kprint_d(va_arg(args, int)); break;
        case 'p': kprint_p(va_arg(args, void*)); break;
        case 'x': kprint_xd(va_arg(args, int)); break;
        case 'c': kprint_c(va_arg(args, char)); break;
        default: flag = false; continue;
      }
      // Reset flag
      flag = false;
    }

    // Advance
    format++;
  }

  // Terminate
  va_end(args);
}

#include "myio.h"

#include "mem.h"

#define SYS_read 0
#define SYS_write 1

// Write using syscall
void my_write(int fp, char* buf, int len) {
  syscall(SYS_write, fp, buf, len);
}

// Read using sycall
void my_read(int fp, char* buf, int len) {
  syscall(SYS_read, fp, buf, len);
}

char* getline() {
  // Cap line length at 255
  char* p = malloc(sizeof(char) * 255);

  int i = 0;
  while (i < 254) {
    char curr;
    my_read(0, &curr, 1);

    // Update for user
    printf("%c", curr);

    p[i] = curr;
    i++;
    
    if (curr == '\n') {
      p[i] = '\0';
      return p;
    }
  }
  p[254] = '\0';
}

/*
   * Printing functions
 */

// Print one char
void print_c(char c) {
   my_write(1, &c, 1); 
}

// Print string
void print_s(const char* str) {
    // Iterate through string and print each char
    while (*str != '\0') {
        print_c(*str);
        str += 1;
    }
}

// Print uint64
void print_d(uint64_t value) {
    // Store digit values
    char buffer[21] = { '\0' };
    int curr = 19;

    // Base case:
    if (value == 0) {
      print_c('0');
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
    print_s(str);
}

// Print hex
void print_x(uint64_t value) {
    // Store values
    char buffer[21] = { '\0' };
    int curr = 19;

    // Base case
    if (value == 0) {
      print_c('0');
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
    print_s(str);
}

// Print pointer
void print_p(void* ptr) {
    print_s("0x");
    print_x((uint64_t)ptr);
}

// Our printf
void printf(const char* format, ...) {
  // Keep track of variable arguments
  va_list args;

  // Initialize
  va_start(args, format);  

  bool flag = false;
  
  // Iterate through string
  while (*format != '\0') {
    // Default case
    if (*format != '%' && !flag) {
      print_c(*format);
    // % case, activates format flag
    } else if (*format == '%' && !flag) {
      flag = true;
    // %% case
    } else if (*format == '%') {
      print_c('%');
      flag = false; 
    } else {
      // Print appropriate arg after % flag
      switch (*format) {
        case 's': print_s(va_arg(args, char*)); break;
        case 'd': print_d(va_arg(args, int)); break;
        case 'p': print_p(va_arg(args, void*)); break;
        case 'x': print_x(va_arg(args, int)); break;
        case 'c': print_c(va_arg(args, char)); break;
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

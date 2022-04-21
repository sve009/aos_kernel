#include "myio.h"
#include "mystring.h"
#include "sys.h"

extern int syscall(uint64_t nr, ...);

// Main shell program
void _start() {
  // Main loop
  while (true) {
    // Set prompt
    printf(">> ");

    // Get line in
    char* line = getline();

    // strip newline
    line[strlen(line)-2] = '\0';

    // Try to execute
    if (exec(line) == -1) {
      printf("Couldn't find %s\n", line);
    }
  }
}

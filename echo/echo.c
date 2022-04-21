#include <myio.h>
#include <sys.h>

void _start() {
  printf("I'll echo what you type:\n");

  // Grab a line
  char* line = getline();

  // Print line back
  printf("%s", line);

  // Exit
  my_exit();
}


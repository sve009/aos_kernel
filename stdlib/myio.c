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

// Get and allocate a line
//   Hard limit of 255 chars for now
char* getline() {
  char* p = malloc(sizeof(char) * 255);

  int i = 0;
  char curr;
  my_read(0, &curr, 1);

  p[i++] = curr;
  
  if (curr == '\n') {
    p[i] = '\0';
    return p;
  }
}

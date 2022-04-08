#pragma once

#include <stdint.h>

extern int syscall(uint64_t nr, ...);

void my_write(int fp, char* buf, int len);

void my_read(int fp, char* buf, int len);

char* getline();

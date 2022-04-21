#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

extern int syscall(uint64_t nr, ...);

void my_write(int fp, char* buf, int len);

void my_read(int fp, char* buf, int len);

char* getline();

void printf(const char* format, ...);

#include "mystring.h"

int strlen(const char* str) {
  int i = 0;
  while (str[i] != '\0') {
    i++;
  }

  return i + 1;
}

int my_strcmp(char* str1, char* str2) {
  int i = 0;
  while (str1[i] != '\0' && str2[i] != '\0') {
    if (str1[i] != str2[i]) {
      return 1;
    }
    i++;
  }
  return str1[i] == str2[i] ? 0 : 1;
}

// Simple copy
void strcpy(char* buf, char* str) {
  int i = 0;
  while (str[i] != '\0') {
    buf[i] = str[i];
    i++;
  }
  i++;
  buf[i] = '\0';
}

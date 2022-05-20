#include "mystring.h"

#include "myio.h"

int strlen(const char* str) {
  int i = 0;
  while (str[i] != '\0') {
    i++;
  }

  return i + 1;
}

void words(char* in, char** out) {
  out[0] = in;
  int len = strlen(in);
  int j = 1;
  for (int i = 0; i < len; i++) {
    if (in[i] == ' ' || in[i] == '\n') {
      out[j++] = &in[i+1];
      in[i] = '\0';
    }
  }
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

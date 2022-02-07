#include <stddef.h>

void* memset(void* s, int c, size_t n) {
  int* mem_area = (int*) s;
  for (int i = 0; i < n; i++) {
    mem_area[i] = c;
  }
  return s;
}

// Retuns the length of a string. (strlen)
int stringlen(const char* str) {
  int result = 0;
  while (str[result] != '\0') {
    result++;
  }
  return result;
}
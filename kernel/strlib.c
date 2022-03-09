#include <stddef.h>
#include <stdint.h>

void* memset(void* s, int c, size_t n) {
  uint8_t* mem_area = (uint8_t*) s;
  for (int i = 0; i < n; i++) mem_area[i] = c;
  return s;
}

// Retuns the length of a string. (strlen)
int stringlen(const char* str) {
  int result = 0;
  while (str[result] != '\0') result++;
  return result;
}

int strcmp(const char *s1, const char *s2) {
  int index = 0;
  while (1) {
    if (s1[index] < s2[index]) return -1;
    if (s1[index] > s2[index]) return 1;
    if (s1[index] == '\0') return 0;
    index++;
  }
}

void* memcpy(void* dest, const void* src, size_t n) {
  uint8_t* dest_ptr = (uint8_t*) dest;
  uint8_t* src_ptr = (uint8_t*) src;
  for (int i = 0; i < n; i++) {
    dest_ptr[i] = src_ptr[i];
  }
  return dest;
}

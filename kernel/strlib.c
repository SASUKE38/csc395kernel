#include <stddef.h>
#include <stdint.h>

/**
 * Set a memory region to a certain byte.
 * \param s Pointer to the start of the region to set.
 * \param c The byte to set.
 * \param n The number of bytes to set (size)
 * \returns A pointer to the region that was set.
 */
void* memset(void* s, int c, size_t n) {
  uint8_t* mem_area = (uint8_t*) s;
  for (int i = 0; i < n; i++) mem_area[i] = c;
  return s;
}

/**
 * Determine the length of a given string.
 * \param str The string whose characters should be counted.
 * \returns The number of characters in str
 */
int stringlen(const char* str) {
  int result = 0;
  while (str[result] != '\0') result++;
  return result;
}

/**
 * Compare two strings lexicographically.
 * \param s1 The first string.
 * \param s2 The second string.
 * \returns 1 if s1 is greater than s2, -1 if s1 is less than s2, 0 if s1 equals s2
 */
int strcmp(const char *s1, const char *s2) {
  int index = 0;
  while (1) {
    if (s1[index] < s2[index]) return -1;
    if (s1[index] > s2[index]) return 1;
    if (s1[index] == '\0') return 0;
    index++;
  }
}

/**
 * Copy bytes from one region to another.
 * \param dest The region to copy bytes to
 * \param src The region to copy bytes from
 * \param n The number of bytes to copy
 * \returns A pointer to the region to copy bytes to (dest)
 */
void* memcpy(void* dest, const void* src, size_t n) {
  uint8_t* dest_ptr = (uint8_t*) dest;
  uint8_t* src_ptr = (uint8_t*) src;
  for (int i = 0; i < n; i++) {
    dest_ptr[i] = src_ptr[i];
  }
  return dest;
}

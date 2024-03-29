#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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
 * Copy a string to a destination. This function also copies the null terminator.
 * src should be null-terminated to avoid illegal memory accesses/buffer overruns.
 * \param dest The destination region to copy the bytes to.
 * \param src The string whose bytes will be copied.
 * \returns A pointer to the destination string
 */
char* strcpy(char* dest, const char* src) {
  for (int i = 0; src[i] != '\0'; i++) dest[i] = src[i];
  return dest;
}

/**
 * Extracts a token from a given string based on determined by a series of delimters. 
 * The returned string is *stringp, but with the first delimiter found replaced with a null terminator.
 * *stringp is then incremented to point to the byte after the found delimiter. If no delimiter is found,
 * then *stringp is set to NULL while the original *stringp is returned.
 * \param stringp Pointer to a string to search for delimters.
 * \param delim The string containing possible delimters.
 * \returns The original pointer, but with the first delimiter replaced with a null terminator (if one was found)
 */
char* strsep(char** stringp, const char* delim) {
  if (*stringp == NULL) return NULL;
  char* result = *stringp;
  bool found = false;
  // Loop over each character in the string until a delimiter is found, incrementing *stringp
  for ( ; **stringp != '\0'; (*stringp)++) {
    // Check if the current character is a delimiter, break if so
    for (int i = 0; delim[i] != '\0'; i++) {
      if (**stringp == delim[i]) {
        found = true;
        **stringp = '\0';
        break;
      }
    }
    // If a character was found, break from the outer loop and add a null terminator
    if (found) {
      //**stringp = '\0';
      break;
    }
  }
  // If no delimiter was found, set *stringp to NULL. Otherwise, set it to point past the delimiter
  if (!found) *stringp = NULL;
  else (*stringp)++;
  return result;
}

/**
 * Locates any character of a given string of characters in a given different string.
 * \param s Pointer to a string to search for characters.
 * \param accept The string containing possible characters.
 * \returns A pointer to the first occurence of a character in accept, or NULL if no character is found
 */
char* strpbrk(const char* s, const char* accept) {
  char* result = (char*) s;
  bool found = false;
  // Loop over the string.
  for ( ; *result != '\0'; result++) {
    // Advance the result pointer until a match is found.
    for (int j = 0; accept[j] != '\0'; j++) {
      if (*result == accept[j]) {
        found = true;
        break;
      }
    }
    // Break out of the outer loop if a match was found.
    if (found) break;
  }
  // Return the advanced pointer if a match was found, NULL otherwise.
  if (!found) return NULL;
  else return result;
}

/**
 * Tokenizes a string based on a given string of delimiters. Removes leading delimiters, then sets the first
 * non-delimiter character in the given string to \0. If no tokens are found, NULL is returned. saveptr is used
 * to store changes between calls; it should not be modified between calls. On every call after the first to parse
 * the same string, str should be NULL.
 * \param str The string to be tokenized. Should be NULL on all but the first call.
 * \param delim The string containing delimiters.
 * \param saveptr Pointer that saves the tokenizing progress between calls. Should not be modified between calls.
 * \returns A pointer to the next token, null-terminated.
 */
char* strtok_r(char* str, const char* delim, char** saveptr) {
  if (str == NULL && saveptr == NULL) return NULL;
  char* str_active = (str == NULL ? *saveptr : str);
  if (stringlen(str_active) == 0) {
    *saveptr = "\0";
    return NULL;
  }
  bool start = false;
  // Loop over the active string's characters
  int num_pos = 0;
  for (int i = 0; i < stringlen(str_active); i++) {
    // Loop over the delimiters. If a current character is a delimiter, 
    // increase the number of positions to advance the start pointer.
    for (int j = 0; j < stringlen(delim); j++) {
      if (str_active[i] == delim[j]) {
        num_pos++;
        break;
      }
      // Stop if the current character was not found in the delimiter string.
      if (j == stringlen(delim) - 1) {
        start = true;
      }
    }
    if (start) break;
  }
  // Return NULL if a start position was not found
  if (!start || num_pos == stringlen(str_active)) {
    *saveptr = "\0";
    return NULL;
  }
  str_active += num_pos;
  
  char* new_start = str_active;
  bool found = false;
  // Loop over the remaining posiitons to find the first delimiter character.
  for ( ; *str_active != '\0'; str_active++) {
    for (int i = 0; delim[i] != '\0'; i++) {
      if (*str_active == delim[i]) {
        found = true;
        *str_active = '\0';
        break;
      }
    }
    if (found) break;
  }
  *saveptr = str_active + 1;
  return new_start;
}

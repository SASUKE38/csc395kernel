#include <unistd.h>
#include <strlib.h>
#include <stddef.h>
#include <stdlib.h>

int printf(const char* format, ...) {
  return 2;
}

// broken
int64_t getline(char** lineptr, size_t* n) {
  if (lineptr == NULL) return -1;
  if (*lineptr == NULL && *n == 0) {
    *lineptr = malloc(sizeof(char) * 1);
    *n += 1;
  }
  int pos = 0;
  while (1) {
    read(0, lineptr[pos], 1);
    if (*lineptr[pos] == '\n') break;
    pos++;
    if (pos == *n) {
      char* new_space = malloc(sizeof(char) * pos + 1);
      memcpy(new_space, *lineptr, pos);
    }
  }
  return pos;
}

void perror(const char *s) {
  write(2, s, stringlen(s));
}
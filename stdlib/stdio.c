#include <unistd.h>
#include <strlib.h>
#include <stddef.h>

int printf(const char *format, ...) {
  return 2;
}

int64_t getline(char **lineptr, size_t *n) {
  return 2;
}

void perror(const char *s) {
  write(2, s, stringlen(s));
}
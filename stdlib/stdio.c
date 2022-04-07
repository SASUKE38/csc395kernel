#include <unistd.h>
#include <strlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>

void print_c(char c) {
  write(1, &c, 1);
}

void print_s(const char* str) {
  int length = stringlen(str);
  write(1, str, length);
}

void print_d(uint64_t value){
  char arr[20];
  int counter = 0;
  for (size_t i = 19; i >= 0; i--) {
    int currentPlace = value%10;
    arr[i] = currentPlace;
    value = value/10;
    if (value == 0){
      counter = i;
      break;
    }
  }

  for(size_t i = counter; i < 20; i++) {
      if (arr[i] >= 0 && arr[i] <= 9) {
      print_c(48 + arr[i]);
    }
  }
}

void print_x(uint64_t value){
  char arr[20];
  int counter = 0;
  for (size_t i = 19; i >= 0; i--) {
    int currentPlace = value%16;
    arr[i] = currentPlace;
    value = value/16;
    if (value == 0){
      counter = i;
      break;
    }
  }

  for(size_t i = counter; i < 20; i++) {
      if (arr[i] >= 0 && arr[i] <= 9) {
      print_c(48 + arr[i]);
    } else {
      print_c(87 + arr[i]);
    }
  }
}

void print_p(void* ptr) {
  uint64_t value = (uint64_t) ptr;
  print_s("0x");
  print_x(value);
}

void printf(const char* format, ...) {
  // Start processing variadic arguments
  va_list args;
  va_start(args, format);

  // Loop until we reach the end of the format string
  size_t index = 0;
  while (format[index] != '\0') {
    // Is the current charater a '%'?
    if (format[index] == '%') {
      // Yes, print the argument
      index++;
      switch(format[index]) {
        case '%':
          print_c('%');
          break;
        case 'c':
          print_c(va_arg(args, int));
          break;
        case 's':
          print_s(va_arg(args, char*));
          break;
        case 'd':
          print_d(va_arg(args, uint64_t));
          break;
        case 'x':
          print_x(va_arg(args, int64_t));
          break;
        case 'p':
          print_p(va_arg(args, void*));
          break;
        default:
          print_s("<not supported>");
      }
    } else {
      // No, just a normal character. Print it.
      print_c(format[index]);
    }
    index++;
  }

  // Finish handling variadic arguments
  va_end(args);
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
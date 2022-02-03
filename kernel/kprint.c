#include "kprint.h"
#include "strlib.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

term_write_t term_write = NULL;

void set_term_write(term_write_t fn) {
    term_write = fn;
}

void kprint_c(char c) {
  term_write(&c, 1);
}

void kprint_s(const char* str) {
  term_write(str, stringlen(str));
}

// Include reference 1
void kprint_d(uint64_t value){
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

  // change to term_write()?
  for(size_t i = counter; i < 20; i++) {
      if (arr[i] >= 0 && arr[i] <= 9) {
      kprint_c(48 + arr[i]);
    }
  }
}

// Include reference 1
void kprint_x(uint64_t value){
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

  // change to term_write()?
  for(size_t i = counter; i < 20; i++) {
      if (arr[i] >= 0 && arr[i] <= 9) {
      kprint_c(48 + arr[i]);
    } else {
      kprint_c(87 + arr[i]);
    }
  }
}

// Change to printing atomically, like storing in an array and printing the array all at once with term_write?
void kprint_p(void* ptr) {
  uint64_t value = (uint64_t) ptr;
  term_write("0x", 2);
  kprint_x(value);
}

void kprintf(const char* format, ...) {
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
          kprint_c('%');
          break;
        case 'c':
          kprint_c(va_arg(args, int));
          break;
        case 's':
          kprint_s(va_arg(args, char*));
          break;
        case 'd':
          kprint_d(va_arg(args, uint64_t));
          break;
        case 'x':
          kprint_x(va_arg(args, int64_t));
          break;
        case 'p':
          kprint_p(va_arg(args, void*));
          break;
        default:
          kprint_s("<not supported>");
      }
    } else {
      // No, just a normal character. Print it.
      kprint_c(format[index]);
    }
    index++;
  }

  // Finish handling variadic arguments
  va_end(args);
}
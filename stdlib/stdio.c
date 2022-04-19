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

/**
* Obtains a line from standard input. Stops reading characters when a newline character is read.
* If *lineptr is NULL or *n is 0, memory is allocated for the buffer and *n is set appropriately.
* If *lineptr was not large enough to hold the line, it is automatically resized. *lineptr should
* be freed when it is no longer used. This version does not include a parameter to specify the input
* file.
* 
* \param lineptr Pointer to a character buffer that will hold the result.
* \param n A pointer to the size of the lineptr buffer.
* \returns The number of characters read, including the newline character.
*/
int64_t getline(char** lineptr, size_t* n) {
  if (*lineptr == NULL || *n == 0) {
    *lineptr = malloc(sizeof(char) * 120);
    *n = 120;
  } /*else if (*lineptr == NULL) {
    *lineptr = malloc(sizeof(char) * *n);
  } */
  // Initialize variables for reading.
  int64_t num_read = -1;
  char current;
  char* cursor = *lineptr;
  // Loop until a newline is read.
  while (1) {
    // Read characters one at a time.
    read(0, &current, 1);
    //write(1, &current, 1);
    num_read++;
    // If the size of *lineptr is equal to or less than 
    // the number of characters read, double the size of the buffer
    // and copy the characters that have been read so far into the new buffer.
    if (*n <= num_read) {
      *n *= 2;
      char* temp = *lineptr;
      *lineptr = malloc(sizeof(char) * *n);
      memcpy(*lineptr, temp, num_read);
      free(temp);
      cursor = *lineptr + num_read;
    }
    *cursor = current;
    cursor++;
    // If the line has reached its end, add a null terminator after the newline 
    // and exit the loop.
    if (current == '\n') {
      cursor[1] = '\0';
      break;
    }
  }
  return (num_read == 0 ? num_read : num_read + 1);
}

/**
* Writes an error message to standard error.
* 
* \param s A string to print.
*/
void perror(const char *s) {
  write(2, s, stringlen(s));
}
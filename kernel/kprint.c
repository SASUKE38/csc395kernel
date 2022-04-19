#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "kprint.h"
#include "strlib.h"
#include "boot.h"
#include "port.h"

// The term_ functions and the following definitions were provided by Professor Curtsinger.
#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15

// Struct representing a single character entry in the VGA buffer
typedef struct vga_entry {
  uint8_t c;
  uint8_t fg : 4;
  uint8_t bg : 4;
} __attribute__((packed)) vga_entry_t;

// A pointer to the VGA buffer
vga_entry_t* term;

// The current cursor position in the terminal
size_t term_col = 0;
size_t term_row = 0;

// Turn on the VGA cursor
void term_enable_cursor() {
  // Set starting scaline to 13 (three up from bottom)
  outb(0x3D4, 0x0A);
  outb(0x3D5, (inb(0x3D5) & 0xC0) | 13);
 
  // Set ending scanline to 15 (bottom)
  outb(0x3D4, 0x0B);
  outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
}

// Update the VGA cursor
void term_update_cursor() {
  uint16_t pos = term_row * VGA_WIDTH + term_col;
 
  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t) (pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

// Clear the terminal
void term_clear() {
  // Clear the terminal
  for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    term[i].c = ' ';
    term[i].bg = VGA_COLOR_BLACK;
    term[i].fg = VGA_COLOR_WHITE;
  }

  term_col = 0;
  term_row = 0;

  term_update_cursor();
}

// Write one character to the terminal
void term_putchar(char c) {
  // Handle characters that do not consume extra space (no scrolling necessary)
  if (c == '\r') {
    term_col = 0;
    term_update_cursor();
    return;

  } else if (c == '\b') {
    if (term_col > 0) {
      term_col--;
      term[term_row * VGA_WIDTH + term_col].c = ' ';
    }
    term_update_cursor();
    return;
  }

  // Handle newline
  if (c == '\n') {
    term_col = 0;
    term_row++;
  }

  // Wrap if needed
  if (term_col == VGA_WIDTH) {
    term_col = 0;
    term_row++;
  }

  // Scroll if needed
  if (term_row == VGA_HEIGHT) {
    // Shift characters up a row
    memcpy(term, &term[VGA_WIDTH], sizeof(vga_entry_t) * VGA_WIDTH * (VGA_HEIGHT - 1));
    term_row--;
    
    // Clear the last row
    for (size_t i=0; i<VGA_WIDTH; i++) {
      size_t index = i + term_row * VGA_WIDTH;
      term[index].c = ' ';
      term[index].fg = VGA_COLOR_WHITE;
      term[index].bg = VGA_COLOR_BLACK;
    }
  }

  // Write the character, unless it's a newline
  if (c != '\n') {
    size_t index = term_col + term_row * VGA_WIDTH;
    term[index].c = c;
    term[index].fg = VGA_COLOR_WHITE;
    term[index].bg = VGA_COLOR_BLACK;
    term_col++;
  }

  term_update_cursor();
}

// Initialize the terminal
void term_init() {
  // Get a usable pointer to the VGA text mode buffer
  term = (vga_entry_t*) phys_to_vir((void*) VGA_BUFFER);

  term_enable_cursor();
  term_clear();
}

/** Prints a character on the terminal. Kernel version.
* \param c The character to print.
*/
void kprint_c(char c) {
  term_putchar(c);
}

/** Prints a string on the terminal. Kernel version.
* \param str The string to print.
*/
void kprint_s(const char* str) {
  int length = stringlen(str);
  for (int i = 0; i < length; i++) term_putchar(str[i]);
}

/*inspiration to use number % base in kprint_d and kprint_x
https://programmerall.com/article/50851418599/
*/
/** Prints an unsigned integer on the terminal. Kernel version.
* \param value The unsigned integer to print.
*/
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

  for(size_t i = counter; i < 20; i++) {
      if (arr[i] >= 0 && arr[i] <= 9) {
      kprint_c(48 + arr[i]);
    }
  }
}

/** Prints an unsigned integer in hexadecimal on the terminal. Kernel version.
* \param value The unsigned integer to print.
*/
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

  for(size_t i = counter; i < 20; i++) {
      if (arr[i] >= 0 && arr[i] <= 9) {
      kprint_c(48 + arr[i]);
    } else {
      kprint_c(87 + arr[i]);
    }
  }
}

/** Prints a pointer on the terminal. Kernel version.
* \param value The pointer to print.
*/
void kprint_p(void* ptr) {
  uint64_t value = (uint64_t) ptr;
  kprint_s("0x");
  kprint_x(value);
}

/** Prints a formatted string on the terminal. Supported format specifiers include:
* %c: char : character
* %d: uint64_t : unsigned 64-bit integer
* %s: const char* : string
* %x: uint64_t : unsigned 64-bit integer in hexadecimal
* %p void* : pointer
* Instances of these in format are replaced by the next variadic argument.
* Kernel version.
* \param format the string to format. Replaces format specifiers with next variadic argument.
*/
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
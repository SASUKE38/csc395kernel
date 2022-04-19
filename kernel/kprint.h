#pragma once

#include <stddef.h>

// Initializes the terminal.
void term_init();

/** Prints a character on the terminal. Kernel version.
* \param c The character to print.
*/
void kprint_c(char c);

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
void kprintf(const char* format, ...);

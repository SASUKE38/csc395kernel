#pragma once

#include <stddef.h>

// Initializes the terminal.
void term_init();

// Print a character
void kprint_c(char c);

// printf with a k
void kprintf(const char* format, ...);

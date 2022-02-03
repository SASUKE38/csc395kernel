#pragma once

#include <stddef.h>

// Set the type of a term_write function
typedef void (*term_write_t)(const char*, size_t);
// Set the terminal writing function
void set_term_write(term_write_t fn);


// printf with a k
void kprintf(const char* format, ...);

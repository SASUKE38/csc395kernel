#pragma once

#include <stdint.h>

// Converts a scan code into a character and prints if appropriate.
void handle_press(uint8_t key_code);

/**
 * Read one character from the keyboard buffer. If the keyboard buffer is empty this function will
 * block until a key is pressed.
 *
 * \returns the next character input from the keyboard
 */
char kgetc();

/**
 * Read a line of characters from the keyboard. Read characters until the buffer fills or a newline
 * character is read. If input ends with a newline, the newline character is stored in output. The
 * string written to output is always null terminated unless the function fails for some reason.
 *
 * \param output A pointer to the beginning of an array where this function should store characters.
 *               This function will write a null terminator into the output array unless it fails.
 * \param capacity The number of characters that can safely be written to the output array
 *                 including the final null termiantor.
 * \returns The number of characters read, or zero if no characters were read due to an error.
 */
size_t kgets(char* output, size_t capacity);

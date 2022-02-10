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

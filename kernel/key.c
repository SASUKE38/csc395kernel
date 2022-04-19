#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "kprint.h"

#define BUFFER_SIZE 2000

// Booleans that determine if a key is being pressed.
bool left_shift = 0;
bool right_shift = 0;
bool num_lock = 0;
bool scroll_lock = 0;
bool caps_lock = 0;
bool left_alt = 0;
bool right_alt = 0;
bool left_control = 0;
bool right_control = 0;

// 147 = unused
// 128 = left control
// 129 = left shift
// 130 = right shift
// 131 = left alt
// 132 = caps lock
// 133-142 = F1-F10
// 143 = num lock
// 144 = scroll lock
// 145 = F11
// 146 = F12

// ASCII character codes indexed by scan code set 1 table
// https://wiki.osdev.org/PS2_Keyboard#Scan_Code_Set_1
uint8_t keys[] = {27 /*escape*/, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48 /*numbers 1-9, 0*/, 
                  45 /*minus*/, 61 /*equals*/, 8 /*backspace*/, 9 /*tab*/, 113, 119, 101, 114, 116,
                  121, 117, 105, 111, 112 /*q-p*/, 91, 93 /*left and right brackets*/, 10 /*enter (newline)*/,
                  128 /*left control*/, 97, 115, 100, 102, 103, 104, 106, 107, 108 /*a-l*/, 59 /*semicolon*/, 
                  39 /*single quote*/, 96 /*back tick (grave, accent)*/, 129 /*left shift*/, 92 /*backslash*/,
                  122, 120, 99, 118, 98, 110, 109 /*z-m*/, 44 /*comma*/, 46 /*period*/, 47 /*forward slash*/,
                  130 /*right shift*/, 42 /*keypad asterisk*/, 131 /*left alt*/, 32 /*space*/, 132 /*caps lock*/,
                  133, 134, 135, 136, 137, 138, 139, 140, 141, 142 /*F1-F10*/, 143 /*Num Lock*/, 144 /*scroll lock*/,
                  55, 56, 57 /*keypad 7-9*/, 45 /*keypad minus*/, 52, 53, 54 /*keypad 4-6*/, 43 /*keypad plus*/,
                  49, 50, 51, 48 /*keypad 1-3, 0*/, 46 /*keypad period*/, 147, 147, 147, 145, 146 /*F11, F12*/};

// ACII character codes indexed by non-capital equivalent in scan code set 1 table. Used for non-alphabetic
// characters pressed when shift is pressed.
uint8_t alternate_keys[] = {27 /*escape*/, 33, 64, 35, 36, 37, 94, 38, 42, 40, 41 /*numbers 1-9, 0*/, 
                  95 /*minus*/, 43 /*equals*/, 8 /*backspace*/, 9 /*tab*/, 113, 119, 101, 114, 116,
                  121, 117, 105, 111, 112 /*q-p*/, 123, 125 /*left and right brackets*/, 10 /*enter (newline)*/,
                  128 /*left control*/, 97, 115, 100, 102, 103, 104, 106, 107, 108 /*a-l*/, 58 /*semicolon*/, 
                  34 /*single quote*/, 126 /*back tick (grave, accent)*/, 129 /*left shift*/, 124 /*backslash*/,
                  122, 120, 99, 118, 98, 110, 109 /*z-m*/, 60 /*comma*/, 62 /*period*/, 63 /*forward slash*/,
                  130 /*right shift*/, 42 /*keypad asterisk*/, 131 /*left alt*/, 32 /*space*/, 132 /*caps lock*/,
                  133, 134, 135, 136, 137, 138, 139, 140, 141, 142 /*F1-F10*/, 143 /*Num Lock*/, 144 /*scroll lock*/,
                  55, 56, 57 /*keypad 7-9*/, 45 /*keypad minus*/, 52, 53, 54 /*keypad 4-6*/, 43 /*keypad plus*/,
                  49, 50, 51, 48 /*keypad 1-3, 0*/, 46 /*keypad period*/, 147, 147, 147, 145, 146 /*F11, F12*/};
                  
// The buffer holding pressed keys.
uint8_t key_buffer[BUFFER_SIZE];
// Position to read from.
int buffer_read = 0;
// Position to write from.
int buffer_write = 0;
// The number of characters in the buffer.
volatile int buffer_count = 0;

/**
 * Adds a character to an externally maintained circular buffer of characters.
 *
 * \param key Character to add to the buffer.
 * \returns The key that was added.
 */
char add_to_buffer(uint8_t key) {
  if (buffer_count != BUFFER_SIZE) {
    key_buffer[buffer_write++] = key;
    buffer_write %= BUFFER_SIZE; // Reset the position if needed
    buffer_count++;
    return key;
  }
  return 0;
}

/**
 * Converts a scan code into a character based on scan code set 1. 
 * The converted character is added to the buffer
 * of characters read from the keyboard.
 * https://wiki.osdev.org/PS2_Keyboard#Scan_Code_Set_1
 */
void handle_press(uint8_t key_code) {
  if (key_code == 0xAA) {
    left_shift = 0;
    return;
  }
  if (key_code == 0xB6) {
    right_shift = 0;
    return;
  }
  if (key_code > 0x58) return;
  uint8_t key = keys[key_code - 1];
  if (key <= 146 && key >= 128) {
    // Handle alt, control, shift, num lock, and scroll lock
    switch (key) {
      case 128:
        left_control = 1;
        break;
      case 129:
        left_shift = 1;
        break;
      case 130:
        right_shift = 1;
        break;
      case 131:
        left_alt = 1;
        break;
      case 132:
        caps_lock = !caps_lock;
        break;
      case 143:
        num_lock = !num_lock;
        break;
      case 144:
        scroll_lock = !scroll_lock;
        break;
      default:
        break;
    }

  } else if (key == 147) {
    kprintf("Unexpected scan code\n");
  } else {
    // handle caps lock
    if (caps_lock && left_shift == 0 && right_shift == 0) {
      // Capitalize the character if it is a letter.
      if (isalpha(key)) {
        add_to_buffer(toupper(key));
        return;
      }
      else { // print a lower case letter if shift is pressed
        add_to_buffer(key);
        return;
      }
    }
    // handle shift
    if (left_shift || right_shift) {
      if (caps_lock) { // don't capitalize if shift and caps lock are pressed
        add_to_buffer(key);
        return;
      }
      // Capitalize the character if it is a letter.
      if (isalpha(key)) {
        add_to_buffer(toupper(key));
        return;
      // Add the key's special character otherwise.
      } else {
        add_to_buffer(alternate_keys[key_code - 1]);
      }
    }
    else {
      add_to_buffer(key);
      return;
    }
  }
}

/**
 * Read one character from the keyboard buffer. If the keyboard buffer is empty this function will
 * block until a key is pressed.
 *
 * \returns the next character input from the keyboard
 */
char kgetc() {
  while (buffer_count == 0) {}
  char result = key_buffer[buffer_read++];
  buffer_read %= BUFFER_SIZE; // Reset the position if needed
  buffer_count--;
  kprint_c(result); // Print the obtained character to get getline and read to print input
  return result;
}

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
size_t kgets(char* output, size_t capacity) {
  char current_char;
  int num_read = 0;
  while (num_read < capacity - 1) {
    current_char = kgetc();
    if (current_char == 0 && num_read == 0) {
      return 0;
    } else if (current_char == '\n') {
      output[num_read++] = current_char;
      break;
    } else {
      output[num_read++] = current_char;
    }
  }
  output[num_read] = '\0';
  return num_read;
}

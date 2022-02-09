#include <stdint.h>
#include <stdbool.h>

#include "kprint.h"
#include "ctype.h"

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

void handle_press(uint8_t key_code) {
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
    if (left_shift || right_shift) {
      if (isalpha(key)) {
        kprintf("%c", toupper(key));
      }
    }
    else {
      kprintf("%c", key);
    }
  }
}

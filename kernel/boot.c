#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "stivale2.h"
#include "util.h"

#include "strlib.h"

// Reserve space for the stack
static uint8_t stack[8192];

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
	.tag = {
    .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
    .next = 0
  },
  .flags = 0
};

// Declare the header for the bootloader
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
  // Use ELF file's default entry point
  .entry_point = 0,

  // Use stack (starting at the top)
  .stack = (uintptr_t)stack + sizeof(stack),

  // Bit 1: request pointers in the higher half
  // Bit 2: enable protected memory ranges (specified in PHDR)
  // Bit 3: virtual kernel mappings (no constraints on physical memory)
  // Bit 4: required
  .flags = 0x1E,
  
  // First tag struct
  .tags = (uintptr_t)&terminal_hdr_tag
};

// Find a tag with a given ID
void* find_tag(struct stivale2_struct* hdr, uint64_t id) {
  // Start at the first tag
	struct stivale2_tag* current = (struct stivale2_tag*)hdr->tags;

  // Loop as long as there are more tags to examine
	while (current != NULL) {
    // Does the current tag match?
		if (current->identifier == id) {
			return current;
		}

    // Move to the next tag
		current = (struct stivale2_tag*)current->next;
	}

  // No matching tag found
	return NULL;
}

typedef void (*term_write_t)(const char*, size_t);
term_write_t term_write = NULL;

void term_setup(struct stivale2_struct* hdr) {
  // Look for a terminal tag
  struct stivale2_struct_tag_terminal* tag = find_tag(hdr, STIVALE2_STRUCT_TAG_TERMINAL_ID);

  // Make sure we find a terminal tag
  if (tag == NULL) halt();

  // Save the term_write function pointer
	term_write = (term_write_t)tag->term_write;
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

// Print usable memory using memmap and hhdm tags
void print_mem_address(struct stivale2_struct* hdr) {
  // Find the hhdm and memmap tags in the list
  struct stivale2_struct_tag_hhdm* hhdm_tag = find_tag(hdr, 0xb0ed257db18cb58f);
  struct stivale2_struct_tag_memmap* memmap_tag = find_tag(hdr, 0x2187f79e8612de07);
  uint64_t memmap_base;
  uint64_t memmap_end;
  uint64_t hhdm_addr = hhdm_tag->addr;

  kprintf("Usable Memory:\n");
  // Loop over the entries of the memmap
  for (int i = 0; i < memmap_tag->entries; i++) {
    // If the entry is usable, process it
    if (memmap_tag->memmap[i].type == 1) {
      // Find the start and end address of the usable range
      memmap_base = memmap_tag->memmap[i].base;
      memmap_end = memmap_base + memmap_tag->memmap[i].length;
      // Print the necessary values and conversions to physical memory.
      kprintf("%p-%p mapped at %p-%p\n", (void*) memmap_base, (void*) memmap_end, (void*) (memmap_base + hhdm_addr), (void*) (memmap_end + hhdm_addr));
    }
  }
}

void _start(struct stivale2_struct* hdr) {
  // We've booted! Let's start processing tags passed to use from the bootloader
  term_setup(hdr);
  print_mem_address(hdr);

	// We're done, just hang...
	halt();
}

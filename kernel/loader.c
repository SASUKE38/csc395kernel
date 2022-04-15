#include <strlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "kprint.h"
#include "page.h"
#include "stivale2.h"
#include "elf.h"
#include "loader.h"
#include "gdt.h"
#include "usermode_entry.h"

void run_exec_elf(char* mod_name, struct stivale2_struct_tag_modules* modules_tag) {
  // Get the number of modules
  uint16_t count = modules_tag->module_count;
  struct stivale2_module* current;
  elf_hdr_t* elf_hdr = NULL;
  elf_phdr_t* elf_phdr = NULL;
  uint16_t phnum;
  int index = 0;
  // Loop over the modules until the requested one is found
  while (index < count) {
    current = &modules_tag->modules[index++];
    if (strcmp(current->string, mod_name) == 0) {
      // Cast the module start address to the elf header
      elf_hdr = (elf_hdr_t*) current->begin;
      // Calculate and store the address of the start of the program header table 
      elf_phdr = (elf_phdr_t*) (current->begin + elf_hdr->e_phoff);
      phnum = elf_hdr->e_phnum;
      break;
    } else if (index >= count) {
      kprintf("Load error: requested file not found in modules\n");
      return;
    }
  }
  unmap_lower_half(read_cr3() & 0xFFFFFFFFFFFFF000);
  // Make sure the file is executable
  if (elf_hdr->e_type != ET_EXEC) {
    kprintf("Load error: attempted to execute non-executable ELF file\n");
    return;
  }
  
  // Loop over the program table entries and allocate memory for loadable segments
  for (int i = 0; i < phnum; i++) {
    if (elf_phdr->p_type == PT_LOAD) {
      // Skip NULL sections
      if (elf_phdr->p_vaddr == 0x0) continue;
      
      // Allocate the segment's requested memory; if it is larger than a page, call vm_map enough times to accomodate the requested chunk
      int64_t size_left = elf_phdr->p_memsz;
      uint64_t vaddr_to_map;
      // If the requested location is not page aligned, set the address to map to the start of the page
      // and increase the size to map by the distance from the start of the previous page
      if (elf_phdr->p_vaddr % PAGE_SIZE != 0) {
        vaddr_to_map =  elf_phdr->p_vaddr - (elf_phdr->p_vaddr % PAGE_SIZE);
        size_left += (elf_phdr->p_vaddr % PAGE_SIZE);
      } else {
        vaddr_to_map = elf_phdr->p_vaddr;
      }
      int i = 0;
      do {
        // Allocate a requested page, setting permissions to writable only for byte copying
        if (vm_map(read_cr3(), vaddr_to_map + (i * PAGE_SIZE), 0, 1, 1) == false) {
          kprintf("Load error: failed to allocate memory for requested page %p\n", elf_phdr->p_vaddr);
          return;
        }
        size_left -= PAGE_SIZE;
        i++;
      } while (size_left > 0);

      // Copy the segment's data into the requested area
      memcpy((void*)elf_phdr->p_vaddr, (const void*) ((uintptr_t) elf_hdr + (uintptr_t) elf_phdr->p_offset), elf_phdr->p_filesz);
      
      // Change the permissions to those requested by the file.
      vm_protect(read_cr3(), elf_phdr->p_vaddr, 1, ((elf_phdr->p_flags & 0x2) >> 1), ~(elf_phdr->p_flags & 0x1) & 0x1);
    }
    elf_phdr++;
  }
  // Pick an arbitrary location and size for the user-mode stack
  uintptr_t user_stack = 0x70000000000;
  size_t user_stack_size = 8 * PAGE_SIZE;

  // Map the user-mode-stack
  for(uintptr_t p = user_stack; p < user_stack + user_stack_size; p += 0x1000) {
    // Map a page that is user-accessible, writable, but not executable
    vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
  }

  // And now jump to the entry point
  usermode_entry(USER_DATA_SELECTOR | 0x3,            // User data selector with priv=3
                  user_stack + user_stack_size - 8,   // Stack starts at the high address minus 8 bytes
                  USER_CODE_SELECTOR | 0x3,           // User code selector with priv=3
                  elf_hdr->e_entry);                  // Jump to the entry point specified in the ELF file
  }

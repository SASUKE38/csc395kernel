// The man page for the official elf.h file was referenced in the creation of this file.
// It contains the essential elements to load and run an executable ELF file.
#pragma once

#include <stdint.h>

// Definitions for e_type
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

// Definitions for p_type
#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6

// Definitions for p_flags
#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

// This struct matches the layout of an ELF file header.
typedef struct elf_hdr {
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff; 
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} __attribute__((packed)) elf_hdr_t;

// This struct matches the layout of an ELF program header.
typedef struct elf_phdr {
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
} __attribute__((packed)) elf_phdr_t;
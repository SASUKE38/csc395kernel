#pragma once

#include "stivale2.h"
#include "boot.h"

#define PAGE_SIZE 0x1000

/**
 * Loads an ELF file from the kernel modules. 
 *
 * \param mod_name The name of the module to load.
 * \param modules_tag A pointer to the stivale2 modules structure.
 * \returns -1 if the requested file was not found, -2 if the file was not executable, 
 * or -3 if the memory allocation failed.
 */
int32_t run_exec_elf(char* mod_name, struct stivale2_struct_tag_modules* modules_tag);

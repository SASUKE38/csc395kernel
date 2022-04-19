#pragma once

#include "stivale2.h"

/**
 * Converts a pointer representing a physical address to its virtual address.
 * \param ptr A pointer to be converted.
 * \returns The converted pointer.
 */
void* phys_to_vir(void* ptr);

/**
 * Obtains the modules tag provided by the bootloader.
 * \returns A pointer to the modules tag.
 */
struct stivale2_struct_tag_modules* get_modules_tag();

#pragma once

#include "stivale2.h"

// Add the hhdm base to a pointer to convert it to a virtual address
void* phys_to_vir(void* ptr);

// Get the pointer to the modules tag stored in the global variable
struct stivale2_struct_tag_modules* get_modules_tag();

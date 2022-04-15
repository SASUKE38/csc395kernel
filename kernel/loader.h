#pragma once

#include "stivale2.h"
#include "boot.h"

#define PAGE_SIZE 0x1000

void run_exec_elf(char* mod_name, struct stivale2_struct_tag_modules* modules_tag);

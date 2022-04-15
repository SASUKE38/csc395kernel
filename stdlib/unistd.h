#pragma once

#include <stdint.h>
#include <stddef.h>

int64_t read(int fd, void *buf, size_t count);

int64_t write(int fd, const void *buf, size_t count);

int64_t exec(char* name);

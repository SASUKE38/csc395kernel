#pragma once

int64_t sys_read(int16_t fd, void *buf, uint16_t count);

int64_t sys_write(int16_t fd, const void *buf, uint16_t count);

int64_t sys_mmap(void* addr, size_t length, int prot, int flags, int fd, uint16_t offset);

int64_t sys_exec(char* name);

int64_t sys_exit();

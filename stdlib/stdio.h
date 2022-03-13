#pragma once

int printf(const char *format, ...);

ssize_t getline(char **lineptr, size_t *n);

void perror(const char *s);
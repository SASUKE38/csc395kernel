#pragma once

#include <stddef.h>

/**
 * Set a memory region to a certain byte.
 * \param s Pointer to the start of the region to set.
 * \param c The byte to set.
 * \param n The number of bytes to set (size)
 * \returns A pointer to the region that was set.
 */
void* memset(void* s, int c, size_t n);

/**
 * Copy bytes from one region to another.
 * \param dest The region to copy bytes to
 * \param src The region to copy bytes from
 * \param n The number of bytes to copy
 * \returns A pointer to the region to copy bytes to (dest)
 */
void* memcpy(void* dest, const void* src, size_t n);

/**
 * Determine the length of a given string.
 * \param str The string whose characters should be counted.
 * \returns The number of characters in str
 */
int stringlen(const char* str);

/**
 * Compare two strings lexicographically.
 * \param s1 The first string.
 * \param s2 The second string.
 * \returns 1 if s1 is greater than s2, -1 if s1 is less than s2, 0 if s1 equals s2
 */
int strcmp(const char* s1, const char* s2);

/**
 * Copy a string to a destination. This function also copies the null terminator.
 * src should be null-terminated to avoid illegal memory accesses/buffer overruns.
 * \param dest The destination region to copy the bytes to.
 * \param src The string whose bytes will be copied.
 * \returns A pointer to the destination string
 */
char* strcpy(char* dest, const char* src);

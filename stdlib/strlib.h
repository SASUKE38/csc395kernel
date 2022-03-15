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

/**
 * Extracts a token from a given string based on determined by a series of delimters. 
 * The returned string is *stringp, but with the first delimiter found replaced with a null terminator.
 * *stringp is then incremented to point to the byte after the found delimiter. If no delimiter is found,
 * then *stringp is set to NULL while the original *stringp is returned.
 * \param stringp Pointer to a string to search for delimters.
 * \param delim The string containing possible delimters.
 * \returns The original pointer, but with the first delimiter replaced with a null terminator (if one was found)
 */
char* strsep(char** stringp, const char* delim);

/**
 * Locates any character of a given string of characters in a given different string.
 * \param s Pointer to a string to search for characters.
 * \param accept The string containing possible characters.
 * \returns A pointer to the first occurence of a character in accept, or NULL if no character is found
 */
char* strpbrk(const char* s, const char* accept);

/**
 * Tokenizes a string based on a given string of delimiters. Removes leading delimiters, then sets the first
 * non-delimiter character in the given string to \0. If no tokens are found, NULL is returned. saveptr is used
 * to store changes between calls; it should not be modified between calls. On every call after the first to parse
 * the same string, str should be NULL.
 * \param str The string to be tokenized. Should be NULL on all but the first call.
 * \param delim The string containing delimiters.
 * \param saveptr Pointer that saves the tokenizing progress between calls. Should not be modified between calls.
 * \returns A pointer to the next token, null-terminated.
 */
char* strtok_r(char* str, const char* delim, char** saveptr);

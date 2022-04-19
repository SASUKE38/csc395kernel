#pragma once

#include <stdint.h>
#include <stddef.h>

/**
* Reads characters from a specified file and places them in a buffer.
* 
* \param fd The file descriptor to read from.
* \param buf A pointer to store read characters in.
* \param count The number of character to read.
* \returns The number of characters read, including the newline character.
*/
int64_t read(int fd, void *buf, size_t count);

/**
* Writes characters from a buffer to a specified file.
* 
* \param fd The file descriptor to read from. Should be 1 or 2.
* \param buf The buffer to write from.
* \param count The number of character to write.
* \returns The number of characters written.
*/
int64_t write(int fd, const void *buf, size_t count);

/**
* Loads a process.
* 
* \param name The name of the process to load.
* \returns -1, since if this point is reached the process was not loaded.
*/
int64_t exec(char* name);

#pragma once

int printf(const char* format, ...);

/**
* Obtains a line from standard input. Stops reading characters when a newline character is read.
* If *lineptr is NULL or *n is 0, memory is allocated for the buffer and *n is set appropriately.
* If *lineptr was not large enough to hold the line, it is automatically resized. *lineptr should
* be freed when it is no longer used.
* 
* \param lineptr Pointer to a character buffer that will hold the result.
* \param n A pointer to the size of the lineptr buffer.
* \returns The number of characters read, including the newline character.
*/
int64_t getline(char** lineptr, size_t* n);

/**
* Writes an error message to standard error.
* 
* \param s A string to print.
*/
void perror(const char* s);
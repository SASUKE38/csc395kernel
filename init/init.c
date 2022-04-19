#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strlib.h>

void _start() {
  // Print a notification that the shell is running
  printf("Shell\n");
  // Declare variables for getline
  char* input;
  size_t input_length;
  // Loop until a valid command is obtained
  do {
    // Set values so getline uses internal sizing for buffer
    input = NULL;
    input_length = 0;
    printf("> ");
    // Read a line
    getline(&input, &input_length);
    // Remove the newline character
    char* input_trunc = strsep(&input, "\n");
    // Skip blank lines
    if (stringlen(input_trunc) == 0) continue;
    int64_t rc = exec(input_trunc);
    // If this point is reached, an error occurred. Print an error message.
    if (rc == -1) printf("Error: requested program not found.\n");
    else if (rc == -2) printf("Error: requested file not executable.\n");
    else if (rc == -3) printf("Error: failed to allocate memory for requested program.\n");
    else printf("Error: failed to execute command.\n");
    
    /* USER MODE TEST
    char* test_page = (char*)0x400000000;
    test_page[0] = 'h';
    test_page[1] = 'e';
    test_page[2] = 'l';
    test_page[3] = 'l';
    test_page[4] = 'o';
    test_page[5] = '\n';
    write(1, test_page, 6);*/
    
  } while (1);
}

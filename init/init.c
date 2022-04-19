#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strlib.h>

void _start() {
  printf("Shell\n");
  char* input;
  size_t input_length;
  do {
    input = NULL;
    input_length = 0;
    printf("> ");
    getline(&input, &input_length);
    char* input_trunc = strsep(&input, "\n");
    if (stringlen(input_trunc) == 0) continue;
    int64_t rc = exec(input_trunc);
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
    //exit(2);
  } while (1);
  /*char* test_page = (char*)0x400000000;
  test_page[0] = 'h';
  test_page[1] = 'e';
  test_page[2] = 'l';
  test_page[3] = 'l';
  test_page[4] = 'o';
  test_page[5] = '\n';
  write(1, test_page, 6);*/

  write(1, "Input to init:\n", 15);
  // Issue a read system call
  char buf[6];
  read(0, buf, 5);

  // Write output
  write(1, "\nread: ", 7);
  write(1, buf, 5);
  write(1, "\n", 1);
  printf("printf tests:\n");
  printf("char: %c\n", 'h');
  printf("integer: %d\n", 45);
  printf("hex: %x\n", 466);
  printf("pointer: %p, hex:%x,int: %d, char: %c\n", 382743, 3847, 45, 'g');
  printf("string: %s\n", "string");
  write(1, "init finished\n", 14);

  

  // Loop forever
  for(;;){}
}

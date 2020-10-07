#ifndef _UI_H_
#define _UI_H_

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WRONG_COMMAND -1
#define COMMAND_EXIT 0
#define COMMAND_PUSH 1
#define COMMAND_POP 2
#define COMMAND_TOP 3
#define COMMAND_EMPTY 4
#define COMMAND_SIZE 5
#define COMMAND_PRINT 6

void print_commands();
void scan_command(char *buf, int size);
bool find_substring(char *where, const char *what);
int check_command(char *a);

#endif

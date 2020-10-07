#ifndef _LIB_H_
#define _LIB_H_

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define WRONG_COMMAND -1
#define COMMAND_EXIT 0
#define COMMAND_SHOW 1
#define COMMAND_FIRST 2
#define COMMAND_LAST 3
#define COMMAND_PUSH 4
#define COMMAND_POP 5
#define COMMAND_LEN 6
#define COMMAND_EMPTY 7

#define COMMAND_PUSH_LENGTH 4

void print_help(const int ostream);
int check_command(char *a);
bool find_substring(char *where, const char *what);
void itoa(int n, char *s); //by K&R
void reverse(char *s);
char *slice(const char *a, const int begin);
void write_string(char *to, const char *from, size_t size);

#endif
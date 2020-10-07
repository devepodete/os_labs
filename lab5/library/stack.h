#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct stack stack;
typedef struct node node;

typedef double data_type;

struct node{
	node *next;
	data_type data;
};

struct stack{
	node *root;
	size_t size;
};

stack *create_stack();
void destroy_stack(stack **s);

bool is_empty(const stack *s);
int size(const stack *s);

void push(stack *s, const data_type value);
void pop(stack *s);

data_type top(const stack *s);

void print(const stack *s);

#endif

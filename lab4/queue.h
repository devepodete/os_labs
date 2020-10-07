#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct node node;
typedef struct queue queue;

struct node{
	node *next;
	node *prev;
	char *value;
};

struct queue{
	node *first;
	node *last;
	size_t size;
};

queue *create_queue();
void destroy_queue(queue **q);

void pop(queue *q);
void push(queue *q, char *data);

char *first(const queue *q);
char *last(const queue *q);

bool is_empty(const queue *q);

size_t length(const queue *q);

void show(queue *q, char *to, size_t max_length);

#endif
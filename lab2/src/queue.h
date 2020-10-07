#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct queue queue;
typedef struct list list;

struct queue{
	list *first;
	list *last;
	int size;
};

struct list{
	list *next;
	list *prev;
	char *value;
};

queue *create();

char *first(const queue *q);
char *last(const queue *q);

void push(queue *q, char *data);
char *pop(queue *q);

bool isEmpty(const queue *q);
int length(const queue *q);

void show(const queue *q, const int ostream);

#endif
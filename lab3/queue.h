#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

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
	pthread_t *thread;
};

void create(queue **q);

void push(queue *q, pthread_t* thread);
void pop(queue *q);

bool isEmpty(const queue *q);
size_t size(const queue *q);

void delete(queue *q);

#endif
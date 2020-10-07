#include "queue.h"
#include "lib.h"

queue *create_queue(){
	queue *q = malloc(sizeof(queue));
	q->first = q->last = NULL;
	q->size = 0;
	return q;
}

void destroy_queue(queue **q){
	while(!is_empty(*q)){
		pop(*q);
	}
	free(*q);
	*q = NULL;
}

void pop(queue *q){
	if(is_empty(q)){
		return;
	}
	if(q->size == 1){
		free(q->first->value);
		q->first->value = NULL;
		free(q->first);
		q->first = NULL;
	}else{
		q->first = q->first->next;
		q->first->prev->next = NULL;
		free(q->first->prev->value);
		q->first->prev->value = NULL;
		free(q->first->prev);
		q->first->prev = NULL;
	}
	q->size--;
}

void push(queue *q, char *data){
	if(data == NULL){
		return;
	}
	if(is_empty(q)){
		q->first = malloc(sizeof(node));
		q->first->next = NULL;
		q->first->prev = NULL;
		q->last = q->first;
		q->first->value = data;
	}else{
		q->last->next = malloc(sizeof(node));
		q->last->next->prev = q->last;
		q->last->next->next = NULL;
		q->last->next->value = data; 
		q->last = q->last->next;
	}	
	q->size++;
}


char *first(const queue *q){
	if(is_empty(q)){
		return NULL;
	}
	return q->first->value;
}

char *last(const queue *q){
	if(is_empty(q)){
		return NULL;
	}
	return q->last->value;
}

bool is_empty(const queue *q){
	return q->size == 0;
}

size_t length(const queue *q){
	return q->size;
}

void show(queue *q, char *to, size_t max_length){
	if(is_empty(q)){
		to[0] = '\0';
		return;
	}
	int i = 0;
	node *temp = q->first;
	while(i < max_length && temp != NULL){
		char *s = temp->value;
		if(i + strlen(s) + 1 > max_length){
			break;
		}else{
			write_string(to + i, s, strlen(s));
			i += strlen(s);
			to[i++] = ' ';
		}
		temp = temp->next;
	}
	to[i] = '\0';
}
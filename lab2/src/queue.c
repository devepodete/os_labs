#include "queue.h"

queue *create(){
	queue *q = malloc(sizeof(queue));
	q->size = 0;
	q->first = q->last = NULL;
	return q;
}

bool isEmpty(const queue *q){
	return q->size == 0;
}

char *pop(queue *q){
	if(isEmpty(q)){
		return 0;
	}
	char *ans = malloc( sizeof(char) * (strlen(q->first->value)+1) );
	strcpy(ans, q->first->value);
	if(q->size == 1){
		free(q->first);
		q->first = NULL;
	}else{
		q->first = q->first->next;
		q->first->prev->next = NULL;
		free(q->first->prev);
		q->first->prev = NULL;
	}
	q->size--;
	return ans;
}

void push(queue *q, char *data){
	if(data == NULL){
		return;
	}
	if(isEmpty(q)){
		q->first = malloc(sizeof(list));
		q->first->next = NULL;
		q->first->prev = NULL;
		q->last = q->first;
		q->first->value = data;
	}else{
		q->last->next = malloc(sizeof(list));
		q->last->next->prev = q->last;
		q->last->next->next = NULL;
		q->last->next->value = data; 
		q->last = q->last->next;
	}	
	q->size++;
}

int length(const queue *q){
	return q->size;
}

void show(const queue *q, const int ostream){
	list *temp = q->first;
	write(ostream, "queue: ", 7);
	while(temp != NULL){
		write(ostream, temp->value, strlen(temp->value));
		write(ostream, " | ", 3);
		temp = temp->next;
	}
	write(ostream, "\n\0", 2);
}

char *first(const queue *q){
	if(isEmpty(q)){
		return "\0";
	}
	return q->first->value;
}

char *last(const queue *q){
	if(isEmpty(q)){
		return "\0";
	}
	return q->last->value;
}
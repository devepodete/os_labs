#include "../stack.h"

stack *create_stack(){
	stack *result = malloc(sizeof(stack));
	if(result != NULL){
		result->root = NULL;
		result->size = 0;
	}
	return result;
}

void destroy_stack(stack **s){
	if(s == NULL){
		return;
	}
	if(*s == NULL){
		return;
	}
	while(!is_empty(*s)){
		pop(*s);
	}
	free(*s);
	*s = NULL;
}

bool is_empty(const stack *s){
	return s->size == 0;
}

int size(const stack *s){
	return s->size;
}

void push(stack *s, const data_type value){
	node *to_push = malloc(sizeof(node));
	if(to_push == NULL){
		printf("Error: can not create node\n");
		return;
	}
	to_push->data = value;
	to_push->next = s->root;
	s->root = to_push;
	s->size++;
}

void pop(stack *s){
	if(is_empty(s)){
		return;
	}else{
		node *temp = s->root->next;
		s->root->next = NULL;
		free(s->root);
		s->root = temp;
		s->size--;
	}
}

data_type top(const stack *s){
	if(is_empty(s)){
		return 0;
	}else{
		return s->root->data;
	}
}

void print(const stack *s){
	if(is_empty(s)){
		printf("<stack is empty>\n");
		return;
	}
	printf("stack: ");
	node *temp = s->root;
	while(temp != NULL){
		if(temp->next == NULL){
			printf("%.3lf", temp->data);
		}else{
			printf("%.3lf, ", temp->data);
		}
		temp = temp->next;
	}
	printf("\n");
}

#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>

#include "../library/stack.h"
#include "../library/ui.h"

int main(){
	void *library_handler = dlopen("../library/libtest.so", RTLD_LAZY);
	if(library_handler == NULL){
		printf("error: can not open dinamic library. errno: %s\n", dlerror());
		return 1;
	}

	stack *(*create_stack)() = dlsym(library_handler, "create_stack");
	void (*destroy_stack)(stack **) = dlsym(library_handler, "destroy_stack");
	bool (*is_empty)(const stack *) = dlsym(library_handler, "is_empty");
	int (*size)(const stack *) = dlsym(library_handler, "size");
	void (*push)(stack *, const data_type) = dlsym(library_handler, "push");
	void (*pop)(stack *) = dlsym(library_handler, "pop");
	data_type (*top)(const stack *) = dlsym(library_handler, "top");
	void (*print)(const stack *) = dlsym(library_handler, "print");

	void (*print_commands)() = dlsym(library_handler, "print_commands");
	void (*scan_command)(char*, const int) = dlsym(library_handler, "scan_command");
	int (*check_command)(char *) = dlsym(library_handler, "check_command");

	stack *s = (*create_stack)();
	if(s == NULL){
		printf("error: can not create stack\n");
		return 1;
	}

	const int max_command_length = 100;
	char command[max_command_length];
	
	(*print_commands)();
	
	bool k = true;
	while(k){
		printf("> ");
		(*scan_command)(command, max_command_length);
		int answer = (*check_command)(command);
		switch(answer){
			case WRONG_COMMAND:
				printf("Error: incorrect command\n");
				break;
			case COMMAND_EXIT:
				k = false;
				break;
			case COMMAND_PUSH:{
				double x;
				scanf("%lf", &x);
				getchar();
				(*push)(s, x);
				break;
			}
			case COMMAND_POP:
				(*pop)(s);
				break;
			case COMMAND_TOP:
				if((*is_empty)(s)){
					printf("<stack is empty>\n");
				}else{
					printf("%lf\n", top(s));
				}
				break;
			case COMMAND_EMPTY:
				if((*is_empty)(s)){
					printf("true\n");
				}else{
					printf("false\n");
				}
				break;
			case COMMAND_SIZE:
				printf("%d\n", (*size)(s));
				break;
			case COMMAND_PRINT:
				(*print)(s);
				break;
			default:
				break;
		}
	}
	(*destroy_stack)(&s);
}


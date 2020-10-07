#include <stdio.h>
#include <stdbool.h>
#include "../library/stack.h"
#include "../library/ui.h"

int main(){
	stack *s = create_stack();
	if(s == NULL){
		printf("Error: can not create stack\n");
		return 1;
	}

	const int max_command_length = 100;
	char command[max_command_length];
	
	print_commands();
	
	bool k = true;
	while(k){
		printf("> ");
		scan_command(command, max_command_length);
		int answer = check_command(command);
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
				push(s, x);
				break;
			}
			case COMMAND_POP:
				pop(s);
				break;
			case COMMAND_TOP:
				if(is_empty(s)){
					printf("<stack is empty>\n");
				}else{
					printf("%lf\n", top(s));
				}
				break;
			case COMMAND_EMPTY:
				if(is_empty(s)){
					printf("true\n");
				}else{
					printf("false\n");
				}
				break;
			case COMMAND_SIZE:
				printf("%d\n", size(s));
				break;
			case COMMAND_PRINT:
				print(s);
				break;
			default:
				break;
		}

	}
	destroy_stack(&s);
}


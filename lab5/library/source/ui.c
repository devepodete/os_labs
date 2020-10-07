#include "../ui.h"

void print_commands(){
	printf("1. exit\n");
	printf("2. push\n");
	printf("3. pop\n");
	printf("4. top\n");
	printf("5. empty\n");
	printf("6. size\n");
	printf("7. print\n");
}

void scan_command(char *buf, const int size){
	int i = 0;
	int c;
	while(i < size-1){
		c = getchar();
		if(c == '\n' || c == '\t' || c == ' ' || c == EOF){
			break;
		}
		buf[i++] = c;
	}
	buf[i] = '\0';
}

bool find_substring(char *where, const char *what){
	const int whereSize = strlen(where);
	const int whatSize = strlen(what);
	
	if(whereSize < whatSize){
		return false;
	}else if(whereSize == whatSize){
		for(int i = 0; i < whereSize; i++){
			if(what[i] != where[i]){
				return false;
			}
		}
		return true;
	}else{
		for(int i = 0; i <= whatSize; i++){
			if(what[i] == '\0' && (where[i] == ' ' || where[i] == '\t')){
				return true;
			}

			if(what[i] != where[i]){
				return false;
			}
		}
		return true;
	}
}

int check_command(char *a){
	char *commands[] = {"exit", "push", "pop", "top", "empty", "size", "print"};
	for(int i = 0; i < 7; i++){
		if(find_substring(a, commands[i]) == true){
			return i;
		}
	}
	return -1;
}

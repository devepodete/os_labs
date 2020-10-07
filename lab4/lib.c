#include "lib.h"

void print_help(const int ostream){
	char *str[] = {
		"Welcome to the UI!\n",
		"List of the commands:\n",
		"0. exit - exit from program\n",
		"1. show - show the queue\n",
		"2. first - top of the queue\n",
		"3. last - back of the queue\n",
		"4. push VALUE - push the VALUE to the queue\n",
		"5. pop - pop the first element from the queue\n",
		"6. len - length of the queue\n",
		"7. empty - is the queue empty?\n",
		"\n-----\nAttention! The limit of input string is 100 symbols!\n-----\n",
		"@"
	};
	for(int i = 0; str[i][0] != '@'; i++){
		write(ostream, str[i], strlen(str[i]));
	}
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
	}
}

int check_command(char *a){
	char *commands[] = {"exit", "show", "first", "last", "push", "pop", "len", "empty"};
	for(int i = 0; i <= 7; i++){
		if(find_substring(a, commands[i]) == true){
			return i;
		}
	}
	return -1; //команда не найдена
}

void itoa(int n, char *s){
	int i, sign;
	if((sign = n) < 0){
		n = -n;
	}
	i = 0;
	do{
		s[i++] = n % 10 + '0';
	}while ((n /= 10) > 0);  
	if(sign < 0){
		s[i++] = '-';
	}
	s[i] = '\0';
	reverse(s);
}

void reverse(char *s){
	int i, j;
	char c;

	for(i = 0, j = strlen(s)-1; i < j; i++, j--){
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}


char *slice(const char *a, const int begin){ //[begin, end]
	if(begin >= strlen(a)){
		return NULL;
	}
	char *result = malloc( sizeof(char) * (strlen(a)-begin+1) );
	int i;
	for(i = begin; i < strlen(a); i++){
		result[i-begin] = a[i];
	}
	result[i-begin] = '\0';
	return result;
}

void write_string(char *to, const char *from, size_t size){
	for(int i = 0; i < size; i++){
		to[i] = from[i];
	}
}
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "queue.h"

void writeCommands(const int ostream);
int checkCommand(const char *a);
bool findSubString(const char *where, const char *what);
void itoa(int n, char *s); //by K&R
void reverse(char *s);
char *slice(const char *a, const int begin, const int end);

int main(){
	//var17
	const int READ = 0;
	const int WRITE = 1;
	
	int toChild[2];
	int toParent[2];
	int error1 = pipe(toChild);
	int error2 = pipe(toParent);
	if(error1 == -1 || error2 == -1){
		write(STDOUT_FILENO, "Can not create pipe(s). Exiting with status \"-1\"\n", 46);
		exit(-1);
	}

	pid_t sp = fork();
	char arr[101]; //массив для записи команд

	if(sp < 0){
		write(STDOUT_FILENO, "Can not create a new process. Exiting with status \"-1\"\n", 53);
		exit(-1);
	}else if(sp > 0){
		//parent

		writeCommands(STDOUT_FILENO);

		while(true){
			write(STDOUT_FILENO, "> ", 2);
			
			int len;
			len = read(STDIN_FILENO, arr, 100); //считывание команды
			arr[len-1] = '\0';
			write(toChild[WRITE], arr, len); //передача команды дочернему процессу

			len = read(toParent[READ], arr, 100); //получение ответа

			if(len > 1){
				write(STDOUT_FILENO, arr, len);
			}else{
				break;
			}
		}
	}else{
		//child
		queue *q = create();
		while(true){
			int len = read(toChild[READ], arr, 100); //считывание команды от родительского процесса
			int x = checkCommand(arr); //проверка корректности команды
			
			if(x == -1){
				write(toParent[WRITE], "Error: incorrect command\n", 25); //ошибка. не найдена команда
			}else{
				switch(x){
					case 0:
						write(toParent[WRITE], "q", 1); //выйти из программы
						break;
					case 1:
						show(q, toParent[WRITE]);
						break;
					case 2:
						if(length(q) > 0){
							write(toParent[WRITE], first(q), strlen(first(q)));
							write(toParent[WRITE], "\n\0", 2);
						}else{
							write(toParent[WRITE], "\0\0", 2);
						}
						break;
					case 3:
						if(length(q) > 0){
							write(toParent[WRITE], last(q), strlen(last(q)));
							write(toParent[WRITE], "\n\0", 2);
						}else{
							write(toParent[WRITE], "\0\0", 2);
						}
						break;
					case 4:
						push(q, slice(arr, 5, strlen(arr)-1)); //5 - "push" + ' '
						write(toParent[WRITE], "\0\0", 2);
						break;
					case 5:
						pop(q);
						write(toParent[WRITE], "\0\0", 2);
						break;
					case 6:{
						char s[100];
						itoa(length(q), s);
						write(toParent[WRITE], s, strlen(s));
						write(toParent[WRITE], "\n\0", 2);
						break;
					}
					case 7:
						if(isEmpty(q)){
							write(toParent[WRITE], "true\n", 5);
						}else{
							write(toParent[WRITE], "false\n", 6);
						}
						break;
				}
			}
		}
	}
	close(toChild[READ]);
	close(toChild[WRITE]);
	close(toParent[READ]);
	close(toParent[WRITE]);
}

void writeCommands(const int ostream){
	write(ostream, "Welcome to the UI!\n", 19);
	write(ostream, "List of the commands:\n", 22);
	write(ostream, "0. exit - exit from program\n", 28);
	write(ostream, "1. show - show the queue\n", 25);
	write(ostream, "2. first - top of the queue\n", 28);
	write(ostream, "3. last - back of the queue\n", 28);
	write(ostream, "4. push VALUE - push the VALUE to the queue\n", 44);
	write(ostream, "5. pop - pop the first element from the queue\n", 46);
	write(ostream, "6. len - length of the queue\n", 29);
	write(ostream, "7. empty - is the queue empty?\n", 31);
	write(ostream,"\n-----\nAttention! The limit of input string is 100 symbols!\n-----\n", 66);
}

bool findSubString(const char *where, const char *what){
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

int checkCommand(const char *a){
	char *commands[] = {"exit", "show", "first", "last", "push", "pop", "len", "empty"};
	for(int i = 0; i <= 7; i++){
		if(findSubString(a, commands[i]) == true){
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


char *slice(const char *a, const int begin, const int end){ //[begin, end]
	if(strlen(a) <= begin){
		return NULL;
	}
	char *result = malloc( sizeof(char) * (end-begin+2) );
	int i;
	for(i = begin; i <= end; i++){
		result[i-begin] = a[i];
	}
	result[i] = '\0';
	return result;
}

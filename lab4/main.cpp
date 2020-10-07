#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h> 
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>

#include "queue.h"
#include "lib.h" //some useful functions for UI

int main(){
	const int ZERO_OFFSET = 0;
	const int UNUSABLE_FILE_DESCRIPTOR = -1;
	const size_t TEXT_MAP_SIZE = 100;
	const size_t ANSWER_MAP_SIZE = 1;

	const int USE_SEMAPHORE_WITH_PROCESSES = 1;
	const unsigned int INITIAL_SEMAPHORE_VALUE = 1;


	print_help(STDOUT_FILENO);

	char *text_map = mmap(NULL,
					TEXT_MAP_SIZE+1,
					PROT_READ | PROT_WRITE,
					MAP_SHARED | MAP_ANONYMOUS,
					UNUSABLE_FILE_DESCRIPTOR,
					ZERO_OFFSET);

	char *answer_map = mmap(NULL,
							ANSWER_MAP_SIZE,
							PROT_READ | PROT_WRITE,
							MAP_SHARED | MAP_ANONYMOUS,
							UNUSABLE_FILE_DESCRIPTOR,
							ZERO_OFFSET);

	answer_map[0] = '1'; //some neutral value

	sem_t *semaphore = sem_open("mySem",
								O_CREAT,
								S_IRGRP | S_IWGRP,
								INITIAL_SEMAPHORE_VALUE);
	//sem_unlink("mySem");
	//sem_close(semaphore);
	//return 0;

	if(semaphore == SEM_FAILED){
		printf("error number %d\n", errno);
		printf("can not create semaphore. exiting with status 1\n");
		exit(1);
	}
	
	pid_t sp = fork();

	if(sp < 0){
		char *ans = "Can not create a new process. Exiting with status \"1\"\n";
		write(STDOUT_FILENO, ans, strlen(ans));
		return 1;
	}else if(sp > 0){
		//parent process

		while(true){
			sem_wait(semaphore);
			
			switch(answer_map[0]){
				case 'b':
					printf("wrong command\n");
					break;
				case 'e':
					goto end;
					break;
				case 'p':
					printf("%s\n", text_map);
					break;
				default:
					break;
			}

			write(STDOUT_FILENO, "> ", 2);
			int len = read(STDIN_FILENO, text_map, TEXT_MAP_SIZE);
			text_map[len-1] = '\0';
			sem_post(semaphore);
			sleep(0.5);
		}
		end:
		
		munmap(text_map, TEXT_MAP_SIZE);
		munmap(answer_map, ANSWER_MAP_SIZE);
		sem_close(semaphore);
		sem_unlink("mySem");
	}else{
		//child process
		queue *q = create_queue();

		while(true){
			sem_wait(semaphore);

			const int cmd = check_command(text_map);
			switch(cmd){
				case WRONG_COMMAND:{
					const char *answer = "b";
					write_string(answer_map, answer, strlen(answer));
					break;
				}
				case COMMAND_EXIT:{
					const char *answer = "e";
					write_string(answer_map, answer, strlen(answer));
					destroy_queue(&q);
					sem_post(semaphore);
					return 0;
				}
				case COMMAND_SHOW:{
					//p for print
					const char *answer = "p";
					write_string(answer_map, answer, strlen(answer));
					show(q, text_map, TEXT_MAP_SIZE);
					break;
				}
				case COMMAND_FIRST:{
					const char *answer = "p";
					write_string(answer_map, answer, strlen(answer));
					char *s = first(q);
					if(s == NULL){
						answer_map[0] = '\0';
					}else{
						write_string(text_map, s, strlen(s));
						text_map[strlen(s)] = '\0';
					}
					break;
				}
				case COMMAND_LAST:{
					const char *answer = "p";
					write_string(answer_map, answer, strlen(answer));
					char *s = last(q);
					if(s == NULL){
						answer_map[0] = '\0';
					}else{
						write_string(text_map, s, strlen(s));
						text_map[strlen(s)] = '\0';
					}
					break;
				}
				case COMMAND_PUSH:{
					const char *answer = "1";
					write_string(answer_map, answer, strlen(answer));
					char *to_push = slice(text_map, COMMAND_PUSH_LENGTH+1);
					push(q, to_push);
					break;
				}
				case COMMAND_POP:{
					const char *answer = "1";
					write_string(answer_map, answer, strlen(answer));
					pop(q);
					break;
				}
				case COMMAND_LEN:{
					const char *answer = "p";
					write_string(answer_map, answer, strlen(answer));
					itoa(length(q), text_map);
					break;
				}
				case COMMAND_EMPTY:{
					const char *answer = "p";
					write_string(answer_map, answer, strlen(answer));
					if(is_empty(q)){
						const char *s = "true";
						write_string(text_map, s, strlen(s));
						text_map[strlen(s)] = '\0';
					}else{
						const char *s = "false";
						write_string(text_map, s, strlen(s));
						text_map[strlen(s)] = '\0';
					}
					break;
				}
			}
			sem_post(semaphore);
			sleep(0.5);
		}
	}
}

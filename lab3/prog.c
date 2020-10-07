#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.h"

struct send{
	double **matrix_;
	double **filter_;
	double **res_;
	size_t matrix_height_, matrix_width_, filter_height_, filter_width_;
	size_t i_matrix, j_matrix;
};

void generate_double_matrix(double ***a, size_t m, size_t n){
	if(a == NULL){
		return;
	}
	free(*a);
	*a = malloc(sizeof(double*)*m);
	for(size_t i = 0; i < m; i++){
		(*a)[i] = malloc(sizeof(double)*n);
	}
}

void copy_matrix(double **from, double **to, size_t h, size_t w){
	for(size_t i = 0; i < h; i++){
		for(size_t j = 0; j < w ; j++){
			to[i][j] = from[i][j];
		}
	}
}

void *foo(void *arg){
	struct send *received = (struct send*)arg;

	int left_border = received->j_matrix - received->filter_width_/2;
	int up_border = received->i_matrix - received->filter_height_/2;
	int right_border = received->j_matrix + received->filter_width_/2;  
	int down_border = received->i_matrix + received->filter_height_/2;

	double sum = 0;

	for(int i = 0; i < received->filter_height_; i++){
		for(int j = 0; j < received->filter_width_; j++){
			if(i + up_border >= received->matrix_height_ || j + left_border >= received->matrix_width_
			|| i + up_border < 0 || j + left_border < 0){
				continue;
			}
			(received->res_)[received->i_matrix][received->j_matrix] += (received->filter_)[i][j]*
				(received->matrix_)[i+up_border][j+left_border];
		}
	}
	free(received);
	return NULL;
}


void use_filter(double **matrix, size_t matrix_height, size_t matrix_width,
				double **filter, size_t filter_height, size_t filter_width,
				double ***res, int max_threads, unsigned k){
	
	if( (matrix_height == 0 || matrix_width == 0 || filter_height == 0 || filter_width == 0) && res != NULL ){
		printf("Error: wrong size of one of the matrices\n");
		*res = NULL;
		return;
	}
	
	if( (matrix == NULL || filter == NULL) && res != NULL ){
		printf("Error: one of the matrix has wrong pointer\n");
		*res == NULL;
		return;
	}

	if(res == NULL){
		printf("Error: wrong result matrix pointer\n");
		return;
	}

	generate_double_matrix(res, matrix_height, matrix_width);

	if(max_threads == 0){
		max_threads = 1;
	}

	for(size_t i = 0; i < matrix_height; i++){
		for(size_t j = 0; j < matrix_width; j++){
			(*res)[i][j] = 0;
		}
	}

	queue *q;
	create(&q);

	if(max_threads < 0){
		//no limit
		for(int count = 0; count < k; count++){
			for(int i = 0; i < matrix_height; i++){
				for(int j = 0; j < matrix_width; j++){
					struct send *s = malloc(sizeof(struct send));
					s->matrix_ = matrix;
					s->filter_ = filter;
					s->matrix_height_ = matrix_height;
					s->matrix_width_ = matrix_width;
					s->filter_height_ = filter_height;
					s->filter_width_ = filter_width;
					s->res_ = *res;
					s->i_matrix = i;
					s->j_matrix = j;
					pthread_t *thr = malloc(sizeof(pthread_t));
					point1:{
						int check = pthread_create(thr, NULL, foo, (void*)s);
						if(check != 0){
							pthread_join(*(q->first->thread), NULL);
							pop(q);
							goto point1;
						}else{
							push(q, thr);
						}	
					}
				}
			}
			while(!isEmpty(q)){
				pthread_join(*(q->first->thread), NULL);
				pop(q);
			}
			copy_matrix(*res, matrix, matrix_height, matrix_width);
		}
	}else{
		//limit
		for(int count = 0; count < k; count++){
			for(size_t i = 0; i < matrix_height; i++){
				for(size_t j = 0; j < matrix_width; j++){
					struct send *s = malloc(sizeof(struct send));
					s->matrix_ = matrix;
					s->filter_ = filter;
					s->matrix_height_ = matrix_height;
					s->matrix_width_ = matrix_width;
					s->filter_height_ = filter_height;
					s->filter_width_ = filter_width;
					s->res_ = *res;
					s->i_matrix = i;
					s->j_matrix = j;
					pthread_t *thr = malloc(sizeof(pthread_t));
					point2:{
						if(size(q) < max_threads){
							int check = pthread_create(thr, NULL, foo, (void*)s);
							if(check != 0){
								pthread_join(*(q->first->thread), NULL);
								pop(q);
								goto point2;
							}else{
								push(q, thr);
							}
						}else{
							pthread_join(*(q->first->thread), NULL);
							pop(q);
							goto point2;
						}
					}
				}
			}
			while(!isEmpty(q)){
				pthread_join(*(q->first->thread), NULL);
				pop(q);
			}
			copy_matrix(*res, matrix, matrix_height, matrix_width);
		}
		
	}
}

void fill_double_matrix(double **a, size_t m, size_t n){
	for(size_t i = 0; i < m; i++){
		for(size_t j = 0; j < n; j++){
			a[i][j] = rand()%100+(double)rand()/RAND_MAX;
		}
	}
}

void print_matrix(double **a, int m, int n){
	for(int i = 0; i < m; i++){
		for(int j = 0; j < n; j++){
			printf("%lf ", a[i][j]);
		}
		printf("\n");
	}
}

void destroy_double_matrix(double ***a, size_t m, size_t n){
	for(int i = 0; i < m; i++){
		free((*a)[i]);
		(*a)[i] = NULL;
	}
	free(*a);
	(*a) = NULL;
}

bool is_int(char *a){
	char *temp = a;
	if(*temp == '-') temp++;
	while(*temp != '\0'){
		if(*temp > '9' || *temp < '0'){
			return false;
		}
		temp++;
	}
	return true;
}

int str_to_int(char *a){
	int sign = 1;
	if(*a == '-'){
		sign = -1;
		a++;
	}
	int res = 0;
	for(char *temp = a; *temp != '\0'; temp++){
		res *= 10;
		res += *temp-'0';
	}
	if(sign == -1){
		res *= -1;
	}
	return res;
}

int main(int argc, char *argv[]){
	srand(time(NULL));
	int max_threads;
	
	if(argc == 1){
		printf("Limit of threads is not set. Default value is 10.\n");
		max_threads = 10;
	}else if(argc == 3){
		if(strcmp(argv[1], "-t") != 0){
			printf("Error: incorrect key\n");
			printf("Usage: %s [-t MAX_THREADS]\n", argv[0]);
			printf("if MAX_THREADS is negative value, then the number of threads is unlimited\n");
			return 1;
		}
		if(!is_int(argv[2])){
			printf("Error: incorrect third argument\n");
			printf("Usage: %s [-t MAX_THREADS]\n", argv[0]);
			printf("if MAX_THREADS is negative value, then the number of threads is unlimited\n");
			return 1;
		}
		max_threads = str_to_int(argv[2]);
	}else{
		printf("Usage: %s [-t MAX_THREADS]\n", argv[0]);
		printf("if MAX_THREADS is negative value, then the number of threads is unlimited\n");
		return 1;
	}
	
	double **matrix = malloc(sizeof(double*));
	double **filter = malloc(sizeof(double*));
	printf("Enter main matrix? y/n: ");
	char ans;
	scanf("%c", &ans);
	getchar();
	size_t matrix_height, matrix_width;
	size_t filter_height, filter_width;
	filter_height = filter_width = 3;

	if(ans == 'y'){
		printf("Enter size: ");
		scanf("%ld %ld", &matrix_height, &matrix_width);
		generate_double_matrix(&matrix, matrix_height, matrix_width);
		printf("Enter matrix: \n");
		for(size_t i = 0; i < matrix_height; i++){
			for(size_t j = 0; j < matrix_width; j++){
				scanf("%lf", &matrix[i][j]);
			}
		}
		getchar();
	}else{
		matrix_height = rand()%200;
		matrix_width = rand()%200;
		generate_double_matrix(&matrix, matrix_height, matrix_width);
		fill_double_matrix(matrix, matrix_height, matrix_width);
		printf("Matrix generated randomly with height = %ld and width = %ld\n", matrix_height, matrix_width);
	}

	generate_double_matrix(&filter, filter_height, filter_width);
	printf("Enter matrix filter? y/n: ");
	scanf("%c", &ans);

	if(ans == 'y'){
		printf("Enter filter:\n");
		for(size_t i = 0; i < filter_height; i++){
			for(size_t j = 0; j < filter_width; j++){
				scanf("%lf", &filter[i][j]);
			}
		}
	}else{
		printf("Filter generated randomly\n");
		fill_double_matrix(filter, filter_height, filter_width);
	}


	unsigned k;
	printf("Enter K: ");
	scanf("%d", &k);
	getchar();

	double **res = malloc(sizeof(double*));
	use_filter(matrix, matrix_height, matrix_width, filter, filter_height, filter_width,
		&res, max_threads, k);
	
	print_matrix(res, matrix_height, matrix_width);

	destroy_double_matrix(&matrix, matrix_height, matrix_width);
	destroy_double_matrix(&filter, filter_height, filter_width);
	destroy_double_matrix(&res, matrix_height, matrix_width);
}

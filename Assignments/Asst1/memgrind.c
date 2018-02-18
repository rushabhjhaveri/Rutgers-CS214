#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<sys/time.h>
#include<ctype.h>
#include "mymalloc.h"

unsigned int DEBUG = 0;

double workloadA(){
	int i = 0;
	double time_taken = 0;
	struct timeval tv1, tv2;
	void* testarr[1000];
	gettimeofday(&tv1, NULL);
	for(i = 0; i < 150; i++){
		testarr[i] = malloc(1);
		if(DEBUG){
			printf("malloced %p\n", &testarr[i]);
		}
		free(testarr[i]);
		if(DEBUG){
			printf("freed\n");
		}
	}

	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workloadA(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

double workloadB(){
	int i = 0;
	double time_taken = 0;
	struct timeval tv1, tv2;
	void* testarr[1000];
	gettimeofday(&tv1, NULL);
	for(i = 0; i < 150; i++){
		testarr[i] = malloc(1);
		if(DEBUG){
			printf("malloced %p\n", &testarr[i]);
		}
	}

	for(i = 0; i < 150; i++){
		free(testarr[i]);
		if(DEBUG){
			printf("freed\n");
		}
	}

	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workloadB(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

double workloadC(){
	int i = 0;
	double time_taken = 0;
	struct timeval tv1, tv2;
	void* testarr[1000];
	int malloc_counter = 0;
	int free_counter = 0;
	int random_bit = 0;
	int size_occupied = 0;
	int random_index = 0;
	gettimeofday(&tv1, NULL);
	while(malloc_counter < 150){
		random_bit = (rand() % 2);
		if(random_bit || size_occupied  == 0){//malloc a new byte
			testarr[size_occupied] = malloc(1);
			malloc_counter++;
			size_occupied++;
			if(DEBUG){
				printf("malloced %p\n", &testarr[size_occupied]);
			}
		}
		else{
			random_index = rand() % size_occupied;
			if(random_index == (size_occupied -1)){
				free(testarr[random_index]);
				testarr[random_index] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}
			else{
				free(testarr[random_index]);
				testarr[random_index] = testarr[size_occupied-1];
				testarr[size_occupied-1] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}

			size_occupied--;
		}
	}
	
	for(i = size_occupied-1; i >= 0; i--){
		free(testarr[i]);
		free_counter++;
		if(DEBUG){
			printf("freed\n");
		}
	}
	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workload C: malloc_counter: %d, free_counter: %d\n", malloc_counter, free_counter);
		printf("In workloadC(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

double workloadD(){
	int i = 0;
	double time_taken = 0;
	struct timeval tv1, tv2;
	void* testarr[5000];
	int malloc_counter = 0;
	int free_counter = 0;
	int random_bit = 0;
	int random_size = 0;
	int size_occupied = 0;
	int random_index = 0;
	gettimeofday(&tv1, NULL);
	while(malloc_counter < 150){
		random_bit = (rand() % 2);
		if(random_bit || size_occupied  == 0){//malloc a new byte
			random_size = ((rand() % 64) + 1);
			testarr[size_occupied] = malloc(random_size);
			malloc_counter++;
			size_occupied++;
			if(DEBUG){
				printf("malloced %p random_size %d \n", &testarr[size_occupied], random_size);
			}
		}
		else{
			random_index = rand() % size_occupied;
			if(random_index == (size_occupied -1)){
				free(testarr[random_index]);
				testarr[random_index] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}
			else{
				free(testarr[random_index]);
				testarr[random_index] = testarr[size_occupied-1];
				testarr[size_occupied-1] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}

			size_occupied--;
		}
	}
	
	for(i = size_occupied-1; i >= 0; i--){
		free(testarr[i]);
		free_counter++;
		if(DEBUG){
			printf("freed\n");
		}
	}
	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workloadD: malloc_counter %d, free_counter: %d\n", malloc_counter, free_counter);
		printf("In workloadD(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

double workloadE(){
	int i = 0;
	double time_taken = 0;
	struct timeval tv1, tv2;
	void* testarr[5000];
	int malloc_counter = 0;
	int free_counter = 0;
	int random_bit = 0;
	int random_size = 0;
	int size_occupied = 0;
	int random_index = 0;
	gettimeofday(&tv1, NULL);
	while(malloc_counter < 50){
		random_bit = (rand() % 2);
		if(random_bit || size_occupied  == 0){//malloc a new byte
			random_size = (rand() > RAND_MAX/2) ? 2 : 3;
			testarr[size_occupied] = malloc(random_size);
			malloc_counter++;
			size_occupied++;
			if(DEBUG){
				printf("malloced %p random_size %d \n", &testarr[size_occupied], random_size);
			}
		}
		else{
			random_index = rand() % size_occupied;
			if(random_index == (size_occupied -1)){
				free(testarr[random_index]);
				testarr[random_index] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}
			else{
				free(testarr[random_index]);
				testarr[random_index] = testarr[size_occupied-1];
				testarr[size_occupied-1] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}

			size_occupied--;
		}
	}
	

	for(i = size_occupied-1; i >= 0; i--){
		free(testarr[i]);
		free_counter++;
		if(DEBUG){
			printf("freed\n");
		}
	}
	
	testarr[size_occupied] = malloc(50);
	malloc_counter++;
	if(DEBUG){
		printf("malloced %p 50\n", &testarr[size_occupied]);
	}
/*	
	while(malloc_counter < 100){
		random_bit = (rand() % 2);
		if(random_bit || size_occupied  == 0){//malloc a new byte
			random_size = (rand() > RAND_MAX/2) ? 2 : 3;
			testarr[size_occupied] = malloc(random_size);
			malloc_counter++;
			size_occupied++;
			if(DEBUG){
				printf("malloced %p random_size %d \n", &testarr[size_occupied], random_size);
			}
		}
		else{
			random_index = rand() % size_occupied;
			if(random_index == (size_occupied -1)){
				free(testarr[random_index]);
				testarr[random_index] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}
			else{
				free(testarr[random_index]);
				testarr[random_index] = testarr[size_occupied-1];
				testarr[size_occupied-1] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}

			size_occupied--;
		}
	}

	for(i = size_occupied-1; i >= 0; i--){
		free(testarr[i]);
		free_counter++;
		if(DEBUG){
			printf("freed\n");
		}
	}
*/	
	free(testarr[size_occupied]);
	free_counter++;
	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workloadE: malloc_counter: %d, free_counter: %d\n", malloc_counter, free_counter);
		printf("In workloadE(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

double workloadF(){
	int i = 0;
	double time_taken = 0;
	struct timeval tv1, tv2;
	void* testarr[5000];
	int malloc_counter = 0;
	int free_counter = 0;
	int random_bit = 0;
	int random_size = 0;
	int size_occupied = 0;
	int random_index = 0;
	gettimeofday(&tv1, NULL);

	for(i = 0; i < 50; i++){
		testarr[i] = malloc(5);
		malloc_counter++;
		if(DEBUG){
			printf("malloced %p 5\n", &testarr[i]);
		}
	}

	while(malloc_counter < 150){
		random_bit = (rand() % 2);
		if(random_bit || size_occupied  == 0){//malloc a new byte
			random_size = ((rand() % 64) + 1);
			testarr[size_occupied] = malloc(random_size);
			malloc_counter++;
			size_occupied++;
			if(DEBUG){
				printf("malloced %p random_size %d \n", &testarr[size_occupied], random_size);
			}
		}
		else{
			random_index = rand() % size_occupied;
			if(random_index == (size_occupied -1)){
				free(testarr[random_index]);
				testarr[random_index] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}
			else{
				free(testarr[random_index]);
				testarr[random_index] = testarr[size_occupied-1];
				testarr[size_occupied-1] = 0;
				free_counter++;
				if(DEBUG){
					printf("freed\n");
				}
			}

			size_occupied--;
		}
	}
	
	for(i = size_occupied-1; i >= 0; i--){
		free(testarr[i]);
		free_counter++;
		if(DEBUG){
			printf("freed\n");
		}
	}

	for(i = 0; i < 50; i++){
		free(testarr[i]);
		free_counter++;
		if(DEBUG){
			printf("freed.\n");
		}
	}

	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workloadF: malloc_counter: %d, free_counter: %d\n", malloc_counter, free_counter);
		printf("In workloadF(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

void error_check(){
	//char arr[5000];
	/*
	int x = 23456;
	free((int *) &x);
	*/

	
	void * p;
	void * q;
	p =  malloc (5000);
	q = malloc(1);
	printf("%p%p",p, q);
	/*
	int a = 10;
	int * x = &a;
	free(x);
	*/




}

int main(){
	error_check();
	//Variable declarations
	/*
	double time_A = 0;
	double time_B = 0;
	double time_C = 0;
	double time_D = 0;
	double time_E = 0;
	double time_F = 0;
	int i = 0;
	
	for(; i < 100; i++){
		//Workload A
		time_A += workloadA();
		//printf("time_A: %f seconds\n", time_A);
	
		//Workload B
		time_B += workloadB();
		//printf("time_B: %f seconds\n", time_B);
	
		//Workload C
		time_C += workloadC();
		//printf("time_C: %f seconds\n", time_C);
	
		//Workload D
		time_D += workloadD();
		//printf("time_D: %f seconds \n", time_D);
	
		//Workload E
		time_E += workloadE();
		//printf("time_E: %f seconds\n", time_E);
	
		//Workload F
		time_F += workloadF();
		//printf("time_F: %f seconds\n", time_F);
	}


	printf("average time_A: %f seconds\n", time_A/100);
	printf("average time_B: %f seconds\n", time_B/100);
	printf("average time_C: %f seconds\n", time_C/100);
	printf("average time_D: %f seconds\n", time_D/100);
	printf("average time_E: %f seconds\n", time_E/100);
	printf("average time_F: %f seconds\n", time_F/100);
	*/
	return 0;
}

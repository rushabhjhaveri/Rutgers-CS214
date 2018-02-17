#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<sys/time.h>
#include<ctype.h>
#include "mymalloc.h"

unsigned int DEBUG = 1;

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
	//int free_counter = 0;
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
				printf("malloced %p\n", &testarr[i]);
			}
		}
		else{
			random_index = rand() % size_occupied;
			if(random_index == (size_occupied -1)){
				free(testarr[random_index]);
				testarr[random_index] = 0;
				if(DEBUG){
					printf("freed\n");
				}
			}
			else{
				free(testarr[random_index]);
				testarr[random_index] = testarr[size_occupied-1];
				testarr[size_occupied-1] = 0;
				if(DEBUG){
					printf("freed\n");
				}
			}

			size_occupied--;
		}
	}
	
	for(i = size_occupied-1; i >= 0; i--){
		free(testarr[i]);
		if(DEBUG){
			printf("freed\n");
		}
	}
	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workloadC(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

double workloadD(){
	int i = 0;
	double time_taken = 0;
	struct timeval tv1, tv2;
	void* testarr[1000];
	int malloc_counter = 0;
	//int free_counter = 0;
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
				printf("malloced %p random_size %d \n", &testarr[i], random_size);
			}
		}
		else{
			random_index = rand() % size_occupied;
			if(random_index == (size_occupied -1)){
				free(testarr[random_index]);
				testarr[random_index] = 0;
				if(DEBUG){
					printf("freed\n");
				}
			}
			else{
				free(testarr[random_index]);
				testarr[random_index] = testarr[size_occupied-1];
				testarr[size_occupied-1] = 0;
				if(DEBUG){
					printf("freed\n");
				}
			}

			size_occupied--;
		}
	}
	
	for(i = size_occupied-1; i >= 0; i--){
		free(testarr[i]);
		if(DEBUG){
			printf("freed\n");
		}
	}
	gettimeofday(&tv2, NULL);
	time_taken = (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

	if(DEBUG){
		printf("In workloadC(), time taken: %f microseconds\n", time_taken*1000000);
	}

	return time_taken;
}

int main(){
	//Variable declarations
	//double time_A = 0;
	//double time_B = 0;
	//double time_C = 0;
	double time_D = 0;
	//double time_E = 0;
	//double time_F = 0;

	//Workload A
	//time_A = workloadA();
	//printf("time_A: %f seconds\n", time_A);
	
	//Workload B
	//time_B = workloadB();
	//printf("time_B: %f seconds\n", time_B);
	
	//Workload C
	//time_C = workloadC();
	//printf("time_C: %f seconds\n", time_C);
	
	//Workload D
	time_D = workloadD();
	printf("time_D: %f seconds \n", time_D);
	return 0;
}

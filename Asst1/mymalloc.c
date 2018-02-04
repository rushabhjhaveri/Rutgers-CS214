#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<math.h>
#include<time.h>
#include "mymalloc.h"

static char myblock[5000];
#define memsize 5000
int DEBUG = 1; // NO DEBUG = 0 DEBUG = 1

metadata * first_metadata = (metadata *) &myblock[0];

char * get_hexaddress(metadata * curr_ptr){

	char * ret_ptr;

	ret_ptr = (char *) curr_ptr;

	ret_ptr = ret_ptr + sizeof(metadata);

	return ret_ptr;
}

metadata * prev_ptr(metadata * curr_ptr){

	char * ret_ptr;
	ret_ptr = (char *) curr_ptr;

	/*
	   Need to go back two steps:
	   1. The current 31-bit block storing the data in the current array index.
	   2. The previous metadata node [because we want to return the pointer at
	   the BEGINNING of the previous metadata node.]
	*/

	ret_ptr = ret_ptr - curr_ptr->block_size - sizeof(metadata);

	return (metadata *) ret_ptr;
}

metadata * next_ptr(metadata * curr_ptr){ //returns a pointer to the next metadata node.

	char * ret_ptr; //Since the array is of type char *
	
	//Make ret_ptr point to current pointer first [LOL legit *almost* forgot this]
	ret_ptr = (char *) curr_ptr;

	/*
	   Need to skip over two things:
	   1. The current metadata node being pointed at.
	   2. The 31-bit block storing the data in the next array index.
	 */

	ret_ptr = ret_ptr + sizeof(metadata) + curr_ptr->block_size;
	return (metadata *) ret_ptr;

}
int if_allocated(metadata *m){ //returns true if array block has been allocated, false otherwise.

	if(m->is_allocated){
		return 1;
	}

	return 0;
}

void * mymalloc(size_t requested_size, char * file, int line_no){

	if(requested_size == 0){ // return null
		return NULL;
	}

	// (requested_size + metadata) >= 5000 check comes here.

	if(requested_size < 0){
		fprintf(stderr, "ERROR: CANNOT ALLOCATE NEGATIVE AMOUNT OF MEMORY.");
		exit(1);
	}

	return NULL;
}

int main(){
	return 0;
}

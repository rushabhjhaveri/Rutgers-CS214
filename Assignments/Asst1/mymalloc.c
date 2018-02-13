#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<math.h>
#include<time.h>
#include "mymalloc.h"

static char myblock[5000];
#define memsize 5000
int DEBUG = 1; // NO DEBUG = 0 DEBUG = 1

header * first_header = (header *) &myblock[0];

// getter function - returns size of header pointer pointing to current block
int get_size(header * curr_ptr){
	return curr_ptr -> block_size;
}

// getter function - returns if current header pointer points to last block in array
int get_islast(header * curr_ptr){
	return curr_ptr -> is_last;
}

char * get_hexaddress(header * curr_ptr){

	char * ret_ptr;

	ret_ptr = (char *) curr_ptr;

	ret_ptr = ret_ptr + sizeof(header);

	return ret_ptr;
}

header * prev_ptr(header * curr_ptr){

	char * ret_ptr;
	ret_ptr = (char *) curr_ptr;

	/*
	   Need to go back two steps:
	   1. The current 31-bit block storing the data in the current array index.
	   2. The previous header node [because we want to return the pointer at
	   the BEGINNING of the previous header node.]
	*/

	ret_ptr = ret_ptr - curr_ptr->prev_block_size - sizeof(header);

	return (header *) ret_ptr;
}

header * next_ptr(header * curr_ptr){ //returns a pointer to the next header node.

	char * ret_ptr; //Since the array is of type char *
	
	//Make ret_ptr point to current pointer first [LOL legit *almost* forgot this]
	ret_ptr = (char *) curr_ptr;

	/*
	   Need to skip over two things:
	   1. The current header node being pointed at.
	   2. The 31-bit block storing the data in the next array index.
	 */

	ret_ptr = ret_ptr + sizeof(header) + curr_ptr->block_size;
	return (header *) ret_ptr;

}
int if_allocated(header *m){ //returns true if array block has been allocated, false otherwise.

	if(m->is_allocated){
		return 1;
	}

	return 0;
}

void * mymalloc(size_t requested_size, char * file, int line_no){

	if(requested_size == 0){ // return null
		return NULL;
	}

	// (requested_size + header) >= 5000 check comes here.

	if((requested_size + sizeof(header)) >= memsize){

		fprintf(stderr, "%s%d ERROR: INTENDED ALLOCATION SIZE EXCEEDS LIMIT", file, line_no);
	}

	if(requested_size < 0){
		fprintf(stderr, "%s%d ERROR: CANNOT ALLOCATE NEGATIVE AMOUNT OF MEMORY.", file, line_no);
		exit(1);
	}
	
	//Start from first block
	header * curr_ptr = first_header;

	//Make sure curr_ptr is within array block
	while((curr_ptr >= first_header) && ((char *) curr_ptr <= &myblock[memsize-1])){

		//check if block is not allocated and make sure size of block 
		//is greater than / equal to required size
		if(!if_allocated(curr_ptr) && get_size(curr_ptr) >= requested_size){
			//Block fits. Allocate memory here.
			int allocated_size = get_size(curr_ptr);
			int allocated_islast = curr_ptr ->is_last;
			//current block has now been allocated. Change its allocation flag
			curr_ptr -> is_allocated = 1;
			//Move to next block
			header * next_block = next_ptr(curr_ptr);

			//Is there enough space left for next block?
			if((get_size(curr_ptr) - requested_size) >= sizeof(header) + 8){
				curr_ptr -> block_size = requested_size;
				header * new_block_header;
				char *temp = (char *) curr_ptr;
				temp += (sizeof(header) + get_size(curr_ptr));
				new_block_header = (header *) temp;

				//Set attributes for new header.
				new_block_header -> is_allocated = 0;
				new_block_header -> block_size = (allocated_size - sizeof(header) -curr_ptr->block_size);
				new_block_header -> is_last = allocated_islast;
				curr_ptr -> is_last = 0;
				new_block_header -> prev_block_size = get_size(curr_ptr);
				next_block -> prev_block_size = get_size(new_block_header);
			} //End inner-if

			return get_hexaddress(curr_ptr);
		} //End main if

		else if(get_islast(curr_ptr)){ //Last block; no match; return error / null
			return NULL;
		}
		else{ //move to next block
			curr_ptr = next_ptr(curr_ptr);
		}
	}//End while
	return NULL;
}

void myfree (void * ptr, char * file, int line_no){

	//Error check - if pointer passed to free is null
	if(ptr == NULL){
		return NULL;
	}
	
	/*
	   Malloc returns a pointer to the address of the array block.
	   We need to make sure the header part [which stores the information regarding
	   cell size, whether allocated, et al.] is also within the array block. Thus,
	   it is sufficient to set the init pointer to the start of the header section.
	*/

	header * init_ptr = (header *) (ptr - sizeof(header));

	//Error check - if pointer is outside array block
	if(init_ptr < first_header || (char *) ptr > &myblock[memsize-1]){
		fprintf(stderr, "%s: %d ERROR: Pointer out of range.\n", file, line_no);
		exit(1);
	}

	//Move variable declaration to the top.
	int allocated = if_allocated(init_ptr);
	if(!allocated){
		fprintf(stderr, "%s: %d ERROR: Memory unallocated. Cannot free unallocated memory.\n");
		exit(1);
	}
	return NULL;
}

int main(){
	return 0;
}

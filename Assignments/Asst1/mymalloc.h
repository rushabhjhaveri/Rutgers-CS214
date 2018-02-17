#ifndef MYMALLOC_H_ //include guard
#define MYMALLOC_H_

typedef struct{
	unsigned int block_size:31; //Size of block
	unsigned int is_allocated:1; //Not allocated = 0; Allocated = 1
	unsigned int prev_block_size:31; //Size of previous block
	unsigned int is_last:1; //Not last block = 0; Last block = 1
} header;

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)

#include<stdlib.h>

int if_allocated(header*);
header * next_ptr(header *);
header * prev_ptr(header *);
void * get_hexaddress(header *);
int get_islast(header *);
int get_size(header *);
void * mymalloc(size_t, char *, int);
void myfree(void *, char *, int);
#endif

#ifndef MYMALLOC_H_ //include guard
#define MYMALLOC_H_

typedef struct{
	unsigned int block_size:31; //Size of block
	unsigned int is_allocated:1; //Not allocated = 0; Allocated = 1
	unsigned int prev_block_size:31; //Size of previous block
	unsigned int is_last:1; //Not last block = 0; Last block = 1
} metadata;

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)


int if_allocated(metadata*);
metadata * next_ptr(metadata *);
metadata * prev_ptr(metadata *);
char * get_hexaddress(metadata *);

#endif

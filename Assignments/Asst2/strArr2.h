#ifndef STRARR2_H_
#define STRARR2_H_

extern int getretArrLen();
extern char **processAndBuildStrArr(char*, int*);     // main routine to      build an dynamic Array of Strings
extern void sort(char **, int );                       // routine to sort     the strings
extern void swap(char **, char **);                    // routine to swap     the 2 string ariguments
extern char ** tokenizer(char *);

extern int getHashMapIndex(char *);

//extern wordNode *isInHashMap(char *, char *, int);

extern void buildHashMap(char*, char *, int);

extern void printHashMap();

extern void doInsertionSort(char *, char *, int);

extern void buildsortedMasterWordList();

extern void printSortedMasterList(char *);


typedef struct strList {
	char *word;
	char *filename;
	int count;
	struct strList* next;
} wordNode;




#endif

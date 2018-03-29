#ifndef INDEX_H_ // Include guard.
#define INDEX_H_

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<ctype.h>
#include<sys/file.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<limits.h>
#include<libgen.h>
#include "strArr2.h"


#define TRUE 1
#define FALSE 0

struct words{
	char * word;
	int wordCount;
	struct 	words * next;
};

struct filenames{
	char * filename;
	struct filenames * next;
	struct words * wordLL;
};

struct stat;

void processAndReadFile(const char *, char *, char *, off_t);
void processAndReadFileFromDir(char *, char *, char *, off_t);
char ** getDistinctWords(char **, int, int *);
void getWordCount(int[], char **, int, char **, int);
void writeToFile(int[], char **, int, const char *, char *);
void listdir(const char *, int);
void processAndReadDir(const char *, char *, char * /*, struct stat*/ );
#endif

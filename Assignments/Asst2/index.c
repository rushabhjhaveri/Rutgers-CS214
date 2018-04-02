#include "index.h"

unsigned int DEBUG = 0; //NO DEBUG = 0; DEBUG = 1
//struct filenames * head = new filenames(NULL, NULL, NULL);
//struct filenames *curr_files, * new_files;
char path[PATH_MAX];


off_t getfilesize(char * filename, struct stat file_stat){
	/*
	if(stat(filename, &file_stat)){
		return 0;
	}
	if(DEBUG){
		printf("In getfilesize: \n");
		printf("filename: %s ", filename);
		printf("filesize: %ld\n", file_stat.st_size);
	}
	return file_stat.st_size;
	*/
	FILE * fp;
	off_t len;
	fp = fopen(filename, "r");
	if(!fp){
		fprintf(stderr, "In getfilesize: File could not be opened.\n");
		exit(0);
	}

	fseek(fp, 0, SEEK_END);
	len = (off_t) ftell(fp);
	fclose(fp);
	return len;
}

void processAndReadFileFromDir(char * filename, char * destFile, char * buffer, off_t filesize){
	/*
	   Variable declarations come here.
	*/
	FILE * fp; /* File pointer */
	char ** sortedArr; /* Contains the sorted array returned by tokenizer. */
	int i, sortedLen; // dwLen; /* sortedLen is the length of sortedArr, dwLen is the length of retdistinctWords. */

	/*
	   If the filename is empty / null
	*/
	if(!filename){
		fprintf(stderr, "ERROR: Invalid\n");
		exit(0);
	}

	fp = fopen(filename, "r"); /*Open the file to be indexed in read mode. */
	
	if(!fp){
		fprintf(stderr, "ERROR: In processAndReadFileFromDir The file %s could not be opened.\n", filename);
		exit(0);
	}

	if(DEBUG){
		printf("In processAndReadFileFromDir, the file: %s  was successfully opened.\n", filename);
	}
	

	// buffer = malloc(filesize * sizeof(char)); /* Allocate space for the buffer. */
	//  buffer = (char *) malloc(filesize * sizeof(char)); /* Allocate space for the buffer. */
 	buffer = (char *) calloc(filesize , sizeof(char)); /* Allocate space for the buffer. */
	size_t bytes;
	bytes = fread(buffer, sizeof(char), filesize, fp); /* Parse through the file to be indexed, store the text in the buffer. */

	if(bytes == 0){
		return;
	}

	if(DEBUG){
		printf("In processAndReadFileFromDir: buffer: %s \n", buffer);
		//printf("%s\n", buffer);
		printf("Buffer has been printed\n");
	}
	
	if(DEBUG){
		printf("Sending file to tokenizer in stringsorter. \n");
	}
	sortedArr = tokenizer(buffer); /* Entire file's text now stored in buffer, pass to tokenizer to tokenize and sort. */
	sortedLen = getretArrLen();

	if(DEBUG){
		printf("In processAndReadFileFromDir, sortedArr: \n");
		for(i = 0; i < sortedLen; i++){
			printf("%s\n", sortedArr[i]);
		}
	}
	
	filename = basename(filename);
	if(DEBUG) printf("BASENAME: %s\n", filename);
	
	int hashIndex = 0;

	for(i = 0; i < sortedLen; i++){
		hashIndex = getHashMapIndex(sortedArr[i]);

		if(DEBUG) {
			printf("In FilesfromDIr:, Word: %s,  hashIndex: %d\n", sortedArr[i], hashIndex);
		}

		buildHashMap(sortedArr[i], filename, hashIndex);
	}
	if(DEBUG)
	printHashMap();

	buildsortedMasterWordList();
		
//	fflush(fp);
	free(buffer);
	fclose(fp);

}

int printWorkingSize(char * file, char * path){
	struct stat file_stat;
	stat(path, &file_stat);
	char result[PATH_MAX];
	sprintf(result, "%lld", (long long)file_stat.st_size);
	printf("Filesize: %s file: %s\n", result, path);
	return 1;
}
		
void processAndReadDir(const char * dirname, char * destFile, char * buffer/*, struct stat file_stat*/){
	

	DIR * root;
	struct dirent * current_entry;
	char * directory_name;
	if (DEBUG) {
		printf("In processAndReadDir(%s)\n", dirname);
	}
	char next_root[PATH_MAX];
	struct stat file_stat;
	
	if ((root = opendir(dirname))) {
	
		while ((current_entry = readdir(root))) {
			directory_name = current_entry->d_name;
			       
			if (strcmp(directory_name, ".") == 0 || strcmp(directory_name, "..") == 0){
				continue;
			} 
            
			sprintf (next_root,"%s/%s", dirname, directory_name);
										                
			stat(next_root,&file_stat);
												                
			if (!S_ISDIR(file_stat.st_mode)) {
					
				processAndReadFileFromDir(next_root, destFile, buffer, file_stat.st_size);
			}
													              
			processAndReadDir(next_root,destFile, buffer);
			
		}
	}

}

void listdir(const char *name, int indent)
{
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(name)))
		return;

	 while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR) {
			char path[1024];
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
		        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
		        printf("%*s[%s]\n", indent, "", entry->d_name);
		        listdir(path, indent + 2);
		} else {
			printf("%*s- %s\n", indent, "", entry->d_name);
	        }
	 }
	
	 closedir(dir);
}

void writeToFile(int wordCount[], char ** distinctWords, int dwLen, const char * filename, char * destFile){

	int fd;
	char ch;
	fd = open(destFile, O_WRONLY | O_CREAT | O_EXCL, 0754);
	if(DEBUG){
		printf("In writeToFile, fd: %d\n", fd);
	}

	if(fd == -1){
		printf("The file you wish to write to already exists. Overwrite? [y/n]: ");
		scanf(" %c", &ch);
		if(DEBUG){
			printf("In writeToFile, ch: %c\n", ch);
		}
		switch(ch){
			case 'y': fd = open(destFile, O_WRONLY | O_TRUNC, 0754);
				  goto write;
				break;
			case 'n': printf("The file will not be overwritten. Exiting...\n");
				  exit(1);
				break;
			default:  fprintf(stderr, "ERROR! Could not interpret choice. Please input y for yes, n for no. Exiting...\n");
				  exit(1);
				break;
		}
	}
	else{
		goto write;
	}

write: 
	if(DEBUG){
		printf("Sucessfully entered write label.\n");
		printf("In write label, fd: %d\n", fd);
	}
	FILE * fp;
	fp = fdopen(fd, "w");
	if(!fp){
		fprintf(stderr, "ERROR! File descriptor could not be converted.\n");
		exit(0);
	}
	int i;
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	
	fprintf(fp, "<fileIndex>\n");
	
	for(i = 0; i < dwLen; i++){
		
		fprintf(fp,"\t<word text = \"%s\">\n", distinctWords[i]);
		fprintf(fp,"\t\t<filename = \"%s\"> %d </file>\n", filename, wordCount[i]);
		fprintf(fp,"\t</word>\n");
	}


	fprintf(fp, "</fileIndex>\n");
}

void getWordCount(int wordCount[],char ** sortedArr, int sortedLen, char ** distinctWords, int dwLen){
	int i, j, retCount = 0;
	//wordCount[dwLen];
	//int * retWordCount;
	for(i = 0; i < dwLen; i++){
		wordCount[i] = 0;
	}
	if(DEBUG){
		printf("In getWordCount: wordCount: \n");
		for(i = 0; i < dwLen; i++){
			printf("wordCount[%d]: %d\n", i, wordCount[i]);
		}
	}
	for(i = 0; i < dwLen; i++){
		for(j = 0; j < sortedLen; j++){
			if(strcmp(distinctWords[i], sortedArr[j]) == 0){
				retCount++;
			}
		}
		wordCount[i] = retCount;
		if(DEBUG){
			printf("In getWordCount, wordCount[%d]: %d\n", i, wordCount[i]);
		}
		retCount = 0;
	}

	/*retWordCount = wordCount;
	if(DEBUG){
		printf("In getWordCount, retWordCount:\n");
		for(i = 0; i < dwLen; i++){
			printf("retWordCount[%d]: %d\n", i, retWordCount[i]);
		}
	}
	return retWordCount; */
}

char ** getDistinctWords(char ** sortedArr, int sortedLen, int * dwLen){

	char ** distinctWords; /* Array of strings representing the distinct words in sortedArr. */
	int i=0, k=0;

	/* Allocate space for distinctWords */
	if(!(distinctWords = malloc(sortedLen * sizeof(*distinctWords)))){
		fprintf(stderr, "ERROR! Malloc failed.\n");
		exit(0);
	}

	while (i < (sortedLen-1)){
		if(DEBUG){
			printf("in getDistinctWords, i: %d\n", i);
			printf("sortedArr[i]: %s\n", sortedArr[i]);
		}
		/* Compare the word at a certain index with the word in the following index. */
		if((strcmp(sortedArr[i], sortedArr[i+1])) != 0){ /* != 0 means the words are not the same, aka, they are distinct. */
			if(!(distinctWords[k] = malloc(strlen(sortedArr[i]) + 1))){
				fprintf(stderr, "ERROR! Malloc of distinctWords[k] failed!\n");
				exit(0);
			}
			else{
				strcpy(distinctWords[k], sortedArr[i]); /* Copy the [distinct] word into distinctWords array. */
				if(DEBUG){
					printf("In getDistinctWords, after strcpy, distinctWords[%d]: %s\n", k, distinctWords[k]);
				}
				k++;
			}
		}
		++i;
	}
	/* This is for the last [distinct] word in sortedArr. */
	distinctWords[k] = malloc(strlen(sortedArr[i]) +1);
	strcpy(distinctWords[k], sortedArr[i]);
	if(DEBUG){
		printf("The last word copied into distinctWords[%d]: %s\n",k, distinctWords[k]);
	}

	/* dwLen is the length of distinctWords array. */
	*dwLen = k+1;
	if(DEBUG){
		printf("In getDistinctWords: \n");
		printf("k: %d\n", k);
		printf("dwLen: %d\n", *dwLen);
		printf("distinctWords: \n");
		for(i = 0; i < *dwLen; ++i){
			printf("%s\n", distinctWords[i]);
		}
	}

	return distinctWords;
}

void processAndReadFile(const char * filename, char * destFile, char * buffer, off_t filesize){
	/*
	   Variable declarations come here.
	*/
	FILE * fp; /* File pointer */
	char ** sortedArr; /* Contains the sorted array returned by tokenizer. */
	char ** retdistinctWords; /* Contains each distinct word in sortedArr. */
	//int  wordCount;
	int i, sortedLen, dwLen; /* sortedLen is the length of sortedArr, dwLen is the length of retdistinctWords. */

	/*
	   If the filename is empty / null
	*/
	if(!filename){
		fprintf(stderr, "ERROR: <Insert error later>\n");
		exit(0);
	}

	fp = fopen(filename, "r"); /*Open the file to be indexed in read mode. */
	
	if(!fp){
		fprintf(stderr, "ERROR: The file %s could not be opened.\n", filename);
		exit(0);
	}

	if(DEBUG){
		printf("The file: %s  was successfully opened.\n", filename);
	}

	buffer = malloc(filesize * sizeof(char)); /* Allocate space for the buffer. */
	size_t bytes;
	
	bytes = fread(buffer, sizeof(char), filesize, fp); /* Parse through the file to be indexed, store the text in the buffer. */

	if(bytes == 0){
		writeToFile(NULL, NULL, 0, filename, destFile);
	}

	if(DEBUG){
		printf("In processAndRead: buffer: \n");
		printf("%s\n", buffer);
	}

	sortedArr = tokenizer(buffer); /* Entire file's text now stored in buffer, pass to tokenizer to tokenize and sort. */
	sortedLen = getretArrLen();

	if(DEBUG){
		printf("In processAndRead, sortedArr: \n");
		for(i = 0; i < sortedLen; i++){
			printf("%s\n", sortedArr[i]);
		}
	}

	

	retdistinctWords = getDistinctWords(sortedArr, sortedLen, &dwLen); /* Get the distinct words from sortedArr. */
	
	if(DEBUG){
		printf("In processAndReadFile: \n");
		printf("dwLen: %d\n", dwLen);
		printf("retdistinctWords: \n");
		for(i = 0; i < dwLen; i++){
			printf("%s\n", retdistinctWords[i]);
		}
	}

	int wordCount[dwLen];
	getWordCount(wordCount, sortedArr, sortedLen, retdistinctWords, dwLen); /* Get number of occurrences of each distinct word in sortedArr. */
	if(DEBUG){
		printf("In processAndRead: wordCount: \n");
			
		for(i = 0; i < dwLen; i++){
			printf("wordCount[%d]: %d\n", i, wordCount[i]);
		}
	}


	writeToFile(wordCount, retdistinctWords, dwLen, filename, destFile); /*Have all the necessary info to produce output file. */

	free(buffer);
	fclose(fp);

}

/*
   Function that returns the size of the file in bytes [aka number of characters].

   @params: filename, system struct stat

   @return: filesize

*/


int main(int argc, char *argv[]){
	/*
	   Variable declarations come here.
	*/
	DIR * dir;
	char * arg;
	char * arg1;
	char buffer[10];
	off_t filesize;
	//struct dirent direct;
	struct stat file_stat;
	/*struct filenames file;
	struct words word;
	struct filenames currFile;
	struct words currWord;
	*/
	int i;
	if(DEBUG){
		printf("argc: %d\n", argc);
	}

	/*
	   Should we add a -h flag? If so, that code comes here.
	*/

	/*
	   Arg check.
	   If arg check fails, exit with failure status.
	*/
	if(argc != 3){
		fprintf(stderr, "%s\n", "ERROR: INCORRECT NUMBER OF ARGUMENTS");
		exit(0);
	}

	/*
	   List out args
	*/
	if(DEBUG){
		for(i = 0; i < argc; i++){
			printf("argv[%d]: %s\n", i, argv[i]);
		}
	}
	arg1 = argv[1];
	arg = argv[2];
	buffer[0] = '\0';

	/*
	   At this point, stringsorter has been modified and works.
	   Ensure rigorous testing to ensure its robustness.
	*/
	
	/*
	   Is argv[2] a file or a directory?
	*/
	dir = opendir(arg);
	if(!dir){ //Is a file.
		filesize = getfilesize(arg, file_stat);
		if(DEBUG){
			printf("filesize: %ld\n", filesize);
		}
		/* Create new file node */
		/*Store filename in created node */
		/* Make head point to newly created file node */
		
		processAndReadFile(arg, arg1, buffer, filesize);
	}
	else{ //is a directory
		//List all the entries in the directory
		if(DEBUG){
			listdir(arg, 0);
		}
		processAndReadDir(arg, arg1, buffer /*, file_stat*/); 
		
		printSortedMasterList(arg1);
		if(closedir(dir) == -1){
			fprintf(stderr, "ERROR! The directory stream could not be closed.\n");
		}

	}
	return 0;
}

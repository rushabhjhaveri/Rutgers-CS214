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

// Prototypes
//char **processAndBuildStrArr(char*, int*);     // main routine to build an dynamic Array of Strings
//void sort(char **, int ); 		       // routine to sort the strings
//void swap(char **, char **);		       // routine to swap the 2 string ariguments

typedef enum { false, true } bool;        //   for boolean variables definition

// global variables
int debug = 0;          // DEBUG = 1 , NO DEBUG = 0
int INIT_WORD_LEN = 10;  // initial word size
int INIT_NO_OF_STRINGS = 10; // initial number of strings in an array
int retArrLen = 0;
// additional structures for testing array of linked list testing
// link list
typedef struct strList {
	char *word;
	char *fileName;
	int count;
	struct strList* next;

} wordNode;


// MAX_HASHMAP_SIZE
#define MAX_HASHMAP_SIZE 31

// word hashMap array of linked list
wordNode *wordHashMap[MAX_HASHMAP_SIZE];

// sorted master word List
wordNode *sortedMasterWordListPtr = NULL;


/*
      New strcmp function - copy and paste into our code. 
      */
int strcmp1(const char * s1, const char * s2){
	const unsigned char * p1 = (const unsigned char *) s1;
	const unsigned char * p2 = (const unsigned char *) s2;
				         
	while(*p1){
		/*
		if(debug){
			printf("p1 = %c \n", *p1);
			printf("p2 = %c \n", *p2);
		}
		*/
		if(*p2 == '\0'){
			return 1;
		}
			
		if(isalpha(*p1) && isdigit(*p2)){
			return -1;
		}
											     
		if(isdigit(*p1) && isalpha(*p2)){
		       	return 1;
		}
															     
		if(isalpha(*p1) && isalpha(*p2)){
		       	if(*p1 < *p2){
				return -1;
			}
		
			else if(*p1 > *p2){
			       	return 1;
			}
		}
													      
		if(isdigit(*p1) && isdigit(*p2)){
			if(*p1 < *p2){
				return -1;
			}
			
			else if(*p1 > *p2){
				return 1;
	        	}

		}
																	       
		if(*p1 == '.' && *p2 != '.'){
			return 1;
		}
																			        
	
		if(*p1 != '.' && *p2 == '.'){
		      	return -1;
		}
																							  
		
		p1++;
		p2++;
	}
					
		if(*p2 != '\0'){
		        return -1;
		}
		   
	return 0;
}



extern int getretArrLen(){
		return retArrLen;
}


extern char **processAndBuildStrArr(char* str, int *retArrLen) {
    // local variables

    char *word;                   // stores the word (string)
    char ch;                      // stores each character from str
    int wordIndex = 0;
    bool wordStarted = false;
    size_t wordLen = INIT_WORD_LEN;

    char **arrOfStrings ;          // will store the array of strings (word)
    int arrIndex = 0;
    size_t noOfStrings = INIT_NO_OF_STRINGS;   // initially value of number of stings in an array


    bool delimsAtStart = true;    // initial set to true, when the first alphabetic character is hit set it to false

    // check the argument received
    if (debug) {
      printf("1.In processAndBuildStrArr: str = [%s]\n",str);
    }

    // allocate space to arrOfStrings  for noOfStrings
    if (! (arrOfStrings = malloc((noOfStrings) * sizeof(*arrOfStrings))) ) {
          printf ("0.In processAndBuildStrArr ERROR: Memory Allocation failure for arrOfStrings\n");
          exit(-1);
    }
    memset(arrOfStrings,'\0', ((noOfStrings) * sizeof(*arrOfStrings)));

   // loop thru the string str and process to build a word String and then an array of strings
   while (*str != '\0') {
       ch = str[0];

       // check if the character is non-alphanumeric and at the beginning of the string
       if ( (delimsAtStart) && (!isalnum(ch)) ) {
          if (debug) {
              printf("2.In processAndBuildStrArr : ch = [%c]\n",ch);
          }
          // ignore the character as it's a separator at the beginning
          str++;         // move the str ptr to next character

       } else if (!isalnum(ch)) {
          // this condition is met when there are delimiters after the word
          // or at the end of the passed string

	    if (wordStarted) {
             word[wordIndex] = '\0';
             if (debug) {
                 printf("6.In processAndBuildStrArr : word = [%s]\n",word);
             }
             wordStarted = false;
             wordIndex = 0;
             // First check if the word begins with a digit, if so ignore it
             if (isdigit(word[0]) ) {   // && (strlen(word) == 1)) {
            	 if (debug) {
            	                  printf("6-A.In processAndBuildStrArr : Ignoring the  word as it begins with a digit -  word = [%s]\n",word);
            	 }
            	 // free the word
            	 free(word);

             // add the word to arrOfStrings at arrIndex
             // check arrIndex against noOfStrings - if < then allocate space for the string and copy the word string
             //                                      if >= then reallocate the arrOfStrings and then copy
             } else if (arrIndex < (noOfStrings -2)) {
                if (! (arrOfStrings[arrIndex] = malloc(strlen(word) + 1)) ) {
                   printf("11. In processAndNuildStrArr : Memory allocation failed for arrOfStrings[arrIndex]\n");
                   exit(-1);
                }
                // copy the word into the arrrOfStrings
                strcpy(arrOfStrings[arrIndex], word);

                if (debug) {
                   printf("12. In processAndNuildStrArr:arrOfStrings[%d]=%s\n",arrIndex,arrOfStrings[arrIndex]);
                }
                arrIndex++;

                // free the word
                free(word);
             } else {

                // reallocate the arrOfStrings and then copy
                noOfStrings *=2;               // double the number of strings each time we reallocate

                if (debug) {
                   printf("13  In else of arrOfStrings allocation arrIndex=[%d]  noOfStrings = [%d]\n",arrIndex,(int) noOfStrings);
                }

                if (! (arrOfStrings =  realloc(arrOfStrings, (noOfStrings * sizeof(*arrOfStrings)) )) ) {
                   printf("13.1 In processAndNuildStrArr : ReAllocation of Memory failed for arrOfStrings[arrIndex]\n");
                   exit(-1);

                }
                // copy the word into the arrrOfStrings
                if (! (arrOfStrings[arrIndex] = malloc(strlen(word) + 1)) ) {
                      printf("13.2. In processAndNuildStrArr : Memory allocation failed for arrOfStrings[arrIndex]\n");
                      exit(-1);
                }
                // copy the word into the arrrOfStrings
                strcpy(arrOfStrings[arrIndex], word);

                if (debug) {
                   printf("13.3. In processAndNuildStrArr:arrOfStrings[%d]=%s\n",arrIndex,arrOfStrings[arrIndex]);
                }
                arrIndex++;

                // free the word
                free(word);

             }
          }  // end if wordStarted

          str++;         // move the str ptr to next character

       } else {

          // alphabetic character encountered - start building the string for here
          if ( (delimsAtStart) || (!wordStarted) ) {
        	 // check if it a digit at the start of  new word...then ignore the character and go to the beginning of the loop
        	  if (isdigit(ch)) {
        		    str++;         // move the str ptr to next character
        		    continue ;         // back to the top of while loop
        	  }
             // if (! (word = (char *)malloc(sizeof(char) * (wordLen+1))) ) {   // same output as next stmt

             if (! (word = (char *)malloc( (wordLen+1))) ) {
                 printf ("3.In processAndBuildStrArr ERROR: Memory Allocation failure \n");
                 exit(-1);
             }
             wordStarted = true;             // set wordStarted to true
             //memset(word,'\0',sizeof(word));   // init the word
             memset(word, '\0',(wordLen+1));
          }

          delimsAtStart = false;             // hit a alphabetic char - change  value to false

          if (debug) {
              printf("4.In processAndBuildStrArr : ch = [%c]\n",ch);
          }

          // add the character to word string
          // check the variable   wordIndex against INIT_WORD_LEN
          if (wordIndex <= (wordLen - 2)) {
             // word array size is withing limits so just push the ch to word array
             word[wordIndex] = ch;
             if (debug) {
                 printf("5.In processAndBuildStrArr : word[%d] = [%c]\n",wordIndex,ch);
             }
             wordIndex++;

          } else {
             //case of extending the word string
             wordLen *=2;
             if (! (word = (char *)realloc(word,(wordLen+1))) ) {
                 printf ("8.In processAndBuildStrArr ERROR: ReAllocation Memory failure \n");
                 exit(-1);
             }
             word[wordIndex] = ch;
             if (debug) {
                 printf("9.In processAndBuildStrArr : word[%d] = [%c]\n",wordIndex,ch);
             }
             wordIndex++;

          }

         str++;         // move the str ptr to next character
       }
   }  // end while

   if (wordStarted) {
       word[wordIndex] = '\0';
       if (debug) {
	   printf("7.In processAndBuildStrArr : word = [%s]\n",word);
       }
       wordStarted = false;
       wordIndex = 0;
       if (isdigit(word[0])) {
       	 if (debug) {
       	                  printf("7-A.In processAndBuildStrArr : Ignoring the  word as it begins with a digit -  word = [%s]\n",word);
       	 }

       	// free the word
       	free(word);

       // add the word to arrOfStrings at arrIndex
       // check arrIndex against noOfStrings - if < then allocate space for the string and copy the word string
       //                                      if >= then reallocate the arrOfStrings and then copy
       } else if (arrIndex < (noOfStrings -2)) {
                if (! (arrOfStrings[arrIndex] = malloc(strlen(word) + 1)) ) {
                   printf("15. In processAndNuildStrArr : Memory allocation failed for arrOfStrings[arrIndex]\n");
                   exit(-1);
                }
                // copy the word into the arrrOfStrings
                strcpy(arrOfStrings[arrIndex], word);

                if (debug) {
                   printf("16. In processAndNuildStrArr:arrOfStrings[%d]=%s\n",arrIndex,arrOfStrings[arrIndex]);
                }
                arrIndex++;

                // free the word
                free(word);
       }  else {

                // reallocate the arrOfStrings and then copy
                noOfStrings *=2;               // double the number of strings each time we reallocate
                if (debug) {
                    printf("17.  In else of arrOfStrings allocation arrIndex=[%d]  noOfStrings = [%d]\n",arrIndex,(int) noOfStrings);
                }
                if (! (arrOfStrings =  realloc(arrOfStrings, (noOfStrings * sizeof(*arrOfStrings)))) ) {
                   printf("18. In processAndNuildStrArr : ReAllocation of Memory failed for arrOfStrings[arrIndex]\n");
                   exit(-1);

                }
                // copy the word into the arrrOfStrings
			    if (! (arrOfStrings[arrIndex] = malloc(strlen(word) + 1)) ) {
					 printf("18.1. In processAndNuildStrArr : Memory allocation failed for arrOfStrings[arrIndex]\n");
					 exit(-1);
			    }
			    // copy the word into the arrrOfStrings
			    strcpy(arrOfStrings[arrIndex], word);

			    if (debug) {
				  printf("18.2. In processAndNuildStrArr:arrOfStrings[%d]=%s\n",arrIndex,arrOfStrings[arrIndex]);
			    }

                arrIndex++;

                // free the word
                free(word);

       }
   }  // end if wordStarted

   if (debug) {
      printf("20.  In processAndNuildStrArr - Just before return \n");
   }
   *retArrLen = arrIndex;
   return(arrOfStrings);

}

extern void swap(char **s1, char **s2){
    char *tmp=*s1;
    *s1=*s2;
    *s2=tmp;
}

extern void sort(char **m, int dim) {
    int i, j, flag=1;

    for(i=0; i<dim-1 && flag==1; i++){
        flag=0;
        for(j=0; j<dim-1; j++)
            if(strcmp1(m[j],m[j+1])>0){
                swap(&m[j],&m[j+1]);
                flag=1;
            }
    }
}

extern int getHashMapIndex(char *str) {
	int index = 0;

	if (debug) {
		printf("getHashMapIndex: str = [%s] \n",str);
	}

	while (*str != '\0'){
		index += (int) str[0];
		str++;

	}
	index = index % MAX_HASHMAP_SIZE ;
	return(index);
}

extern wordNode *isInHashMap(char *wordStr , char* fileName, int hashIndex) {
	// return value isInHashMap
	wordNode *retWordNodePtr = NULL;

	wordNode *tempWordNodePtr = wordHashMap[hashIndex];
	while (tempWordNodePtr != NULL){
		if (  (strcmp(tempWordNodePtr->word,wordStr) == 0) && ((strcmp(tempWordNodePtr->fileName,fileName) == 0))) {
				retWordNodePtr = tempWordNodePtr;
				break;
		}
		tempWordNodePtr = tempWordNodePtr->next;
	}

	return 	retWordNodePtr;
}

extern void buildHashMap(char *wordStr , char* fileName, int hashIndex) {

	// return value isInHashMap
	wordNode *retWordNodePtr = NULL;

	// temporary wordNode Ptr
	wordNode *temp;

	// create a new word node
	wordNode *newWordNodePtr =  (wordNode *) malloc(sizeof(wordNode) );

	newWordNodePtr->word =(char*)malloc(sizeof(wordStr));
	newWordNodePtr->fileName =(char*)malloc(sizeof(fileName));
	strcpy(newWordNodePtr->word,wordStr);
	strcpy(newWordNodePtr->fileName,fileName);
	newWordNodePtr->count = 1;
	newWordNodePtr->next = NULL;

	if (debug){
		printf("buildHashMap-0: newWordNodePtr->word=[%s] newWordNodePtr->fileName=[%s] newWordNodePtr->count=[%d]\n",
				                  newWordNodePtr->word,newWordNodePtr->fileName,newWordNodePtr->count);
	}

	if (wordHashMap[hashIndex] == NULL){
		// it's the first word node at hashIndex
		wordHashMap[hashIndex]  = newWordNodePtr;

		if (debug){
				printf("buildHashMap-1: wordHashMap[hashIndex]->word=[%s] wordHashMap[hashIndex]->fileName=[%s] wordHashMap[hashIndex]->count=[%d]\n",
						wordHashMap[hashIndex]->word,wordHashMap[hashIndex]->fileName, wordHashMap[hashIndex]->count);
		}

	} else {
	     retWordNodePtr = isInHashMap(wordStr,fileName,hashIndex);
		 if (retWordNodePtr == NULL){
			 // the word is not there in the linked list
			 // so add it at the beginning of the hashIndex
                 temp = wordHashMap[hashIndex];
                 newWordNodePtr->next = temp;
                 wordHashMap[hashIndex] = newWordNodePtr;
         		if (debug){
         				printf("buildHashMap-2: wordHashMap[hashIndex]->word=[%s] wordHashMap[hashIndex]->fileName=[%s] wordHashMap[hashIndex]->count=[%d]\n",
         						   wordHashMap[hashIndex]->word,wordHashMap[hashIndex]->fileName, wordHashMap[hashIndex]->count);
         			}


		 } else {
			 // the word is present in the word list at hashIndex
			 // so just increment the count of retWordNodePtr->count
			 retWordNodePtr->count++;
			// continue;
      		if (debug){
      				printf("buildHashMap-3: retWordNodePtr->word=[%s] retWordNodePtr->fileName=[%s] retWordNodePtr->count=[%d]\n",
      						retWordNodePtr->word,retWordNodePtr->fileName, retWordNodePtr->count);
      		}

		 }

	}


}

extern void printHashMap() {
	wordNode *tempWordNodePtr;

	for (int hashIndex = 0; hashIndex < MAX_HASHMAP_SIZE ; hashIndex++) {
		if (wordHashMap[hashIndex] == NULL) {
			//printf("printHashMap: wordHashMap[%d] = NULL\n",hashIndex);
			//do nothing
			continue;
		} else {
			// run thru the list at index i
			tempWordNodePtr = wordHashMap[hashIndex];
			if(debug){
				printf("printHAshMap: wordHashMap[%d]= \n",hashIndex);			
				printf("==================================\n");
			}
			while (tempWordNodePtr != NULL){
				printf(" tempWordNodePtr->word=[%s] tempWordNodePtr->fileName=[%s] tempWordNodePtr->count=[%d]\n",
						      tempWordNodePtr->word,tempWordNodePtr->fileName,tempWordNodePtr->count);
				tempWordNodePtr = tempWordNodePtr->next;
			}

		}
	}

}


void doInsertionSort(char *wordStr, char*fileName,int count) {

	char * p = wordStr;
	for(; *p; ++p){
		*p = tolower(*p);
	}

	p = fileName;
	for(; *p; ++p){
		*p = tolower(*p);
	}

	bool insertDone = false;                  // set to true if the insert is done within the while loop
	int result, res0;
	// prev , current and next wordNodePtr
	wordNode *currentWordNodePtr;
	wordNode *nextWordNodePtr;
	wordNode *prevWordNodePtr;
	// create a new word node
	wordNode *newWordNodePtr =  (wordNode *) malloc(sizeof(wordNode) );
	newWordNodePtr->word =(char*)malloc(sizeof(wordStr));
	newWordNodePtr->fileName =(char*)malloc(sizeof(fileName));
	strcpy(newWordNodePtr->word,wordStr);
	strcpy(newWordNodePtr->fileName,fileName);
	newWordNodePtr->count = count;
	newWordNodePtr->next = NULL;

	if (sortedMasterWordListPtr == NULL) {
		sortedMasterWordListPtr = newWordNodePtr;
		insertDone = true;
	} else {
		currentWordNodePtr = sortedMasterWordListPtr;
		prevWordNodePtr = sortedMasterWordListPtr;
		nextWordNodePtr = sortedMasterWordListPtr->next;
		
		while (currentWordNodePtr !=NULL){
		 	
			//compare the word string
			res0 = strcmp1(wordStr, currentWordNodePtr->word);
			if(debug){
				printf("0. In insertionSort, result of comparing wordStr [%s] with currentWordNode [%s]: %d\n", wordStr, currentWordNodePtr->word, res0);
				printf("1, In insertionSort, filename [%s] with currentWordNode filename [%s]: \n", newWordNodePtr->fileName, currentWordNodePtr->fileName);
			}
			if(res0 == 0){
				// compare the counts
				if (currentWordNodePtr->count == newWordNodePtr->count) {
					result = strcmp1(currentWordNodePtr->fileName, newWordNodePtr->fileName);

					if(debug){
						printf("2. Insertion Sort : Comparing: %s and %s\n",  currentWordNodePtr->fileName, newWordNodePtr->fileName);
						printf("3. Insertion Sort : result of comparision: %d\n", result);
					}
			
					if(/*strcmp1(currentWordNodePtr->fileName, newWordNodePtr->fileName)*/ result == -1){
						
					//	printf("Before: %s and %s\n",  currentWordNodePtr->fileName, newWordNodePtr->fileName);
						currentWordNodePtr->next = newWordNodePtr;
						if(nextWordNodePtr){
							newWordNodePtr->next = nextWordNodePtr;
						}
						if(debug){
							printf("4. Insertion : word count is same, current < new\n");
							printf("5. Inertion :: After: %s and %s\n",  currentWordNodePtr->fileName, newWordNodePtr->fileName);

						}
					}
					else if(/*strcmp1(currentWordNodePtr->fileName, newWordNodePtr->fileName)*/ result == 1){
						if(prevWordNodePtr == currentWordNodePtr){
							prevWordNodePtr = newWordNodePtr;
							newWordNodePtr->next = currentWordNodePtr;
							sortedMasterWordListPtr = newWordNodePtr;
						}
						else{
							if(prevWordNodePtr){
								prevWordNodePtr->next = newWordNodePtr;
							}
							newWordNodePtr->next = currentWordNodePtr;
						}
					}
					else{
						//just update the count
						(currentWordNodePtr->count)++;
						//update currentWordNode
						currentWordNodePtr = currentWordNodePtr->next;
						//break;
					}
					insertDone = true;
					break;
				} else if (currentWordNodePtr->count < count) {
					if (currentWordNodePtr == sortedMasterWordListPtr) {
						// add at the head of the list
						newWordNodePtr->next = currentWordNodePtr;
						sortedMasterWordListPtr = newWordNodePtr;
						insertDone = true;
						break;
					} else {
						// add new Node before the current Node
						newWordNodePtr->next = currentWordNodePtr;
						prevWordNodePtr->next = newWordNodePtr;
						insertDone = true;
						break;
					}
				} else if (currentWordNodePtr->count > count) {
					// add new Node after the current Node
					if (nextWordNodePtr == NULL) {
						// add at the end of the current
						currentWordNodePtr->next = newWordNodePtr;
						insertDone = true;
						break;
					} else {
						// move to the next node as there may be many nodes with the same string and higher counts
						prevWordNodePtr = currentWordNodePtr;
						currentWordNodePtr = currentWordNodePtr->next;
						if (currentWordNodePtr->next !=NULL) {
							nextWordNodePtr = currentWordNodePtr->next;
						} else {
							nextWordNodePtr = NULL;
						}
					}
				}
			} else if (/*(strcmp1(wordStr,currentWordNodePtr->word) < 0)*/ res0 == -1) {
				// add before current Node
				if (currentWordNodePtr == sortedMasterWordListPtr) {
					// add at the head of the list
					newWordNodePtr->next = currentWordNodePtr;
					sortedMasterWordListPtr = newWordNodePtr;
					insertDone = true;
					break;
				} else {
					// add new Node before the current Node
					newWordNodePtr->next = currentWordNodePtr;
					prevWordNodePtr->next = newWordNodePtr;
					insertDone = true;
					break;
				}
			} else if (/*(strcmp1(wordStr,currentWordNodePtr->word) > 0)*/ res0 == 1) {
				// new word string is after current node , so mode to the next node to compare
				prevWordNodePtr = currentWordNodePtr;
				currentWordNodePtr = currentWordNodePtr->next;
				if (currentWordNodePtr !=NULL) {
					nextWordNodePtr = currentWordNodePtr->next;
				} else {
					nextWordNodePtr = NULL;
				}
			}
		}  // end while
		if (! insertDone) {
			// implies it's to be added to the end of the master list
			prevWordNodePtr->next = newWordNodePtr;
		}
	}
}


extern void buildsortedMasterWordList() {
	sortedMasterWordListPtr = NULL;
	wordNode *tempWordNodePtr;

		for (int hashIndex = 0; hashIndex < MAX_HASHMAP_SIZE ; hashIndex++) {
			if (wordHashMap[hashIndex] != NULL) {
				// run thru the list at index i
				tempWordNodePtr = wordHashMap[hashIndex];
				
				if(debug){
					printf("In buildMaster, filename = %s\n", tempWordNodePtr->fileName);
				}
				while (tempWordNodePtr != NULL){

						doInsertionSort(tempWordNodePtr->word,tempWordNodePtr->fileName,tempWordNodePtr->count);
						tempWordNodePtr = tempWordNodePtr->next;
				}

			}
		}
}

extern void printSortedMasterList(char * destFile) {
	
	if(debug){
		printf("printSortedMasterList \n");
		printf("==================================\n");
	}

	int fd;
	char ch;
	fd = open(destFile, O_WRONLY | O_CREAT | O_EXCL, 0754);
	if(debug){
		printf("In writeToFile, fd: %d\n", fd);
	}

	if(fd == -1){
		printf("The file you wish to write to already exists. Overwrite? [y/n]: ");
		scanf(" %c", &ch);
		if(debug){
			printf("In printSortedMasterList, ch: %c\n", ch);
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
	if(debug){
		printf("Sucessfully entered write label.\n");
		printf("In write label, fd: %d\n", fd);
	}
	
	FILE * fp;
	fp = fdopen(fd, "w");
	
	wordNode *tempWordNodePtr;
	tempWordNodePtr = sortedMasterWordListPtr;
	
	bool printWord = true;

	if(!fp){
		fprintf(stderr, "ERROR! File descriptor could not be converted.\n");
		exit(0);
	}
	//int i;
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
										
	fprintf(fp, "<fileIndex>\n");										

	char *filename;
	char *word;
	int count;

	if(tempWordNodePtr == NULL){
		fprintf(fp, "</fileIndex>\n");
	}
	

	while (tempWordNodePtr != NULL){
		
		filename = tempWordNodePtr->fileName;
		word = tempWordNodePtr->word;
		count = tempWordNodePtr->count;
		
		//tempWordNodePtr = prevWordNodePtr->next;

		if(debug){		
			printf("In printMaster, tempWordNodePtr->word=[%s] tempWordNodePtr->fileName=[%s] tempWordNodePtr->count=[%d]\n", word, filename, count);
		}

		if(printWord){
			fprintf(fp, "\t<word text = \"%s\">\n", tempWordNodePtr->word);
			fprintf(fp, "\t\t<file name = \"%s\"> %d </file>\n", tempWordNodePtr->fileName, tempWordNodePtr->count);
			printWord = false;
		}
		else{
			if(tempWordNodePtr->next == NULL){
				fprintf(fp, "\t</word>\n");
				fprintf(fp, "</fileIndex>\n");
				break;
			}
			else{
				if(strcmp1(tempWordNodePtr->word, tempWordNodePtr->next->word) != 0){
					fprintf(fp, "\t</word>\n");
					tempWordNodePtr = tempWordNodePtr->next;
					printWord = true;
				}
				else{
					fprintf(fp, "\t\t<file name = \"%s\"> %d </file>\n", tempWordNodePtr->next->fileName, tempWordNodePtr->next->count);
					tempWordNodePtr = tempWordNodePtr->next;
				}
			}
		}

	}
	

}

extern char **  tokenizer (char * buffer) {
	
	char **retArrofStrings;
	if (strlen(buffer) == 0 ) {
		//printf("ERROR : Empty String\n");
		exit(1);
	}
	
	retArrofStrings=processAndBuildStrArr(buffer, &retArrLen);
	if (!retArrofStrings) {
		printf ("ERROR : In main : Unable to allicate retArrofStrings\n");
	}

	if (debug) {
		printf("In main - Value of retArrLen = [%d]\n",retArrLen);
	}

	   // call the sort function
	sort(retArrofStrings,retArrLen);

	if (debug) {
		printf("In main - After calling sort\n");
	}

	int tmpi;
	int i;

	if(debug){
		for (tmpi = 0; tmpi < retArrLen ; tmpi++) {
		printf("%s\n",retArrofStrings[tmpi]);
		}
	}

	for(tmpi = 0; tmpi < retArrLen; tmpi++){
		i = 0;
		while(retArrofStrings[tmpi][i] != '\0'){
			retArrofStrings[tmpi][i] = tolower(retArrofStrings[tmpi][i]);
			i++;
		}
	}

	return retArrofStrings;
}

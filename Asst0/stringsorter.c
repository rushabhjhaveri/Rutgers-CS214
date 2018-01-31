#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "stringsorter.h"

typedef enum { false, true } bool;        //   for boolean variables definition

// global variables
int DEBUG = 0;          // DEBUG = 1 , NO DEBUG = 0
int INIT_WORD_LEN = 10;  // initial word size
int INIT_NO_OF_STRINGS = 10; // initial number of strings in an array

char **processAndBuildStrArr(char* str, int *retArrLen) {
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
    if (DEBUG) {
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

       // check if the character is non-alphabetic and at the beginning of the string
       if ( (delimsAtStart) && (!isalpha(ch)) ) {
          if (DEBUG) {
              printf("2.In processAndBuildStrArr : ch = [%c]\n",ch);
          }
          // ignore the character as it's a separator at the beginning
          str++;         // move the str ptr to next character

       } else if (!isalpha(ch)) {
          // this condition is met when there are delimiters after the word
          // or at the end of the passed string

          if (wordStarted) {
             word[wordIndex] = '\0';
             if (DEBUG) {
                 printf("6.In processAndBuildStrArr : word = [%s]\n",word);
             }
             wordStarted = false;
             wordIndex = 0;

             // add the word to arrOfStrings at arrIndex
             // check arrIndex against noOfStrings - if < then allocate space for the string and copy the word string
             //                                      if >= then reallocate the arrOfStrings and then copy
             if (arrIndex < (noOfStrings -2)) {
                if (! (arrOfStrings[arrIndex] = malloc(strlen(word) + 1)) ) {
                   printf("11. In processAndNuildStrArr : Memory allocation failed for arrOfStrings[arrIndex]\n");
                   exit(-1);
                }
                // copy the word into the arrrOfStrings
                strcpy(arrOfStrings[arrIndex], word);

                if (DEBUG) {
                   printf("12. In processAndNuildStrArr:arrOfStrings[%d]=%s\n",arrIndex,arrOfStrings[arrIndex]);
                }
                arrIndex++;

                // free the word
                free(word);
             } else {

                // reallocate the arrOfStrings and then copy
                noOfStrings *=2;               // double the number of strings each time we reallocate

                if (DEBUG) {
                   printf("13  In else of arrOfStrings allocation arrIndex=[%d]  noOfStrings = [%d]\n",arrIndex,noOfStrings);
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

                if (DEBUG) {
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

          if (DEBUG) {
              printf("4.In processAndBuildStrArr : ch = [%c]\n",ch);
          }

          // add the character to word string
          // check the variable   wordIndex against INIT_WORD_LEN
          if (wordIndex <= (wordLen - 2)) {
             // word array size is withing limits so just push the ch to word array
             word[wordIndex] = ch;
             if (DEBUG) {
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
             if (DEBUG) {
                 printf("9.In processAndBuildStrArr : word[%d] = [%c]\n",wordIndex,ch);
             }
             wordIndex++;

          }

         str++;         // move the str ptr to next character
       }
   }  // end while

   if (wordStarted) {
       word[wordIndex] = '\0';
       if (DEBUG) {
	   printf("7.In processAndBuildStrArr : word = [%s]\n",word);
       }
       wordStarted = false;
       wordIndex = 0;
       // add the word to arrOfStrings at arrIndex
       // check arrIndex against noOfStrings - if < then allocate space for the string and copy the word string
       //                                      if >= then reallocate the arrOfStrings and then copy
       if (arrIndex < (noOfStrings -2)) {
                if (! (arrOfStrings[arrIndex] = malloc(strlen(word) + 1)) ) {
                   printf("15. In processAndNuildStrArr : Memory allocation failed for arrOfStrings[arrIndex]\n");
                   exit(-1);
                }
                // copy the word into the arrrOfStrings
                strcpy(arrOfStrings[arrIndex], word);

                if (DEBUG) {
                   printf("16. In processAndNuildStrArr:arrOfStrings[%d]=%s\n",arrIndex,arrOfStrings[arrIndex]);
                }
                arrIndex++;

                // free the word
                free(word);
       }  else {

                // reallocate the arrOfStrings and then copy
                noOfStrings *=2;               // double the number of strings each time we reallocate
                if (DEBUG) {
                    printf("17.  In else of arrOfStrings allocation arrIndex=[%d]  noOfStrings = [%d]\n",arrIndex,noOfStrings);
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

			    if (DEBUG) {
				  printf("18.2. In processAndNuildStrArr:arrOfStrings[%d]=%s\n",arrIndex,arrOfStrings[arrIndex]);
			    }

                arrIndex++;

                // free the word
                free(word);

       }
   }  // end if wordStarted

   if (DEBUG) {
      printf("20.  In processAndNuildStrArr - Just before return \n");
   }
   *retArrLen = arrIndex;
   return(arrOfStrings);

}

void swap(char **s1, char **s2){
    char *tmp=*s1;
    *s1=*s2;
    *s2=tmp;
}

void sort(char **m, int dim) {
    int i, j, flag=1;

    for(i=0; i<dim-1 && flag==1; i++){
        flag=0;
        for(j=0; j<dim-1; j++)
            if(strcmp(m[j],m[j+1])>0){
                swap(&m[j],&m[j+1]);
                flag=1;
            }
    }
}

int main (int argc, char** argv) {


    char **retArrofStrings;
    int retArrLen = 0;

    if (DEBUG) {
     printf("The fol. arguments are passed to main()\n");
     for (int i = 0; i < argc; i++) {
       printf("[%s] :", argv[i]);
     }
     printf("\n");
  } // END DEBUG


  if (argc != 2) {
     //printf("ERROR : Invalid no. of arguments passed to the program\n");
     printf("Usgae : stringsorter \"<a string>\"\n");
     exit(1);
  }

  if (strlen(argv[1]) == 0 ) {
     //printf("ERROR : Empty String\n");
     exit(1);
  }

  retArrofStrings=processAndBuildStrArr(argv[1], &retArrLen);
  if (!retArrofStrings) {
      printf ("ERROR : In main : Unable to allicate retArrofStrings\n");
  }

  if (DEBUG) {
     printf("In main - Value of retArrLen = [%d]\n",retArrLen);
  }

  /*for (int tmpi = 0; tmpi < retArrLen ; tmpi++) {
      printf("retArrofStrings[%d]=[%s]\n",tmpi,retArrofStrings[tmpi]);
  }*/

  // call the sort function
  sort(retArrofStrings,retArrLen);

  if (DEBUG) {
     printf("In main - After calling sort\n");
  }

  for (int tmpi = 0; tmpi < retArrLen ; tmpi++) {
      printf("%s\n",retArrofStrings[tmpi]);
  }


}

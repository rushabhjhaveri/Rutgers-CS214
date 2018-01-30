/*
======================================
Class: Systems Programming [CS214]

Semester: Spring 2018

Assignment 0

Collaborator Name       NetID

Hetalben Patel          hp373

Rushabh Jhaveri         rrj28
=========================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "stringsorter.h"

unsigned int DEBUG = 1; // NO DEBUG = 0 ; DEBUG = 1

char *trimwhitespace(char *str)
{
	if(DEBUG){
		printf("string entering trim: %s\n", str);
	}

	  char *end;

	    // Trim leading space
//	    while(!isalpha((unsigned char)*str)) str++;

	    while((!isalpha((unsigned char)*str)) &&  strlen(str) != 0) {
		    if(DEBUG){
			    printf("Str in first  while is : %s", str);
		    }
		    str++;

	    }


	      if(*str == 0){  // All spaces?
		      if(DEBUG){
			      printf("Returning null char \n");
		      }
		         // return '\0';
		          return str;
	      }

	        // Trim trailing space
	        end = str + strlen(str) - 1;
		  //while(end > str && isspace((unsigned char)*end)) end--;
		  while(end > str && (!isalpha((unsigned char)*end))) end--;

		    // Write new null terminator
		    *(end+1) = '\0';
		    if(DEBUG){
			    printf("string leaving trim: \"%s\" : %d\n", str, (int) strlen(str));
		    }

		      return str;
}
/*
char** build_words(char *wordsarr[], char *string, int len, int arraylen){
	if(DEBUG){
		printf("Entered build_words \n");
	}
	int i = 0;
	int j = 0;
	int k = 0;
	size_t word_size = 10; //average word length
	size_t sz_mem = word_size+1; //memory required
	char *word = malloc(sz_mem * sizeof(*word)); //allocate memory for word
	if(!word){ //validate memory created successfullu, or throw error
		fprintf(stderr, "ERROR: Virtual memory exhausted allocating 'word'\n");
		exit(0);
	}
	memset(word, '\0', sz_mem); //initialize memory to zero
	char ch = '0';
	char *storedword;

	if(DEBUG){
		printf("arraylen: %d\n", arraylen);
	}

	for(i = 0; i < len; i++){
		ch = string[i];
		if(DEBUG){
			printf("ch: %c\n", ch);
		}
		if(isalpha(ch)){
			//append(word, ch);
			if(DEBUG){
				printf("0.Word: \"%s\"\n", word);
			}
			word[k] = ch;
			if(k == (strlen(word) - 3)){ //realloc - make word twice as big
				void *temp = realloc(word, 2*sz_mem); //use a temporary pointer
				if(!temp){ //check if realloc succeeded
					fprintf(stderr, "ERROR: Virtual memory exhausted, realloc 'word' \n");
					exit(0);
				}

				memset(temp + sz_mem, 0, sz_mem * sizeof(*word)); //zero new memory
				word = temp; //assign new block to word
				sz_mem += sz_mem; //update current allocation size
			}
			if(DEBUG){
				printf("1.Word: %s\n", word);
			}
			k++;
		}
		else{ //non-alphabetic character => word built
			//dump word in array
			if(DEBUG){
				printf("2.Word: %s\n", word);
			}
			storedword = (char*) malloc(strlen(word)+1);
			strcpy(storedword, word);
			wordsarr[j] = storedword;
			
			if(DEBUG){
				printf("wordsarr[%d]: %s\n", j, wordsarr[j]);
			}
			//memset(word, '\0', sizeof(*word)) ;
			word[0] = '\0';
			if(DEBUG){
				printf("3.Word:\"%s\"\n", word);
			}
			k = 0;
			j++;
		}
	} // end for

	if(isalpha(ch)){
		storedword = (char*) malloc(strlen(word)+1);
		strcpy(storedword, word);
		wordsarr[j] = storedword;
		if(DEBUG){
			printf("wordsarr[%d]: %s\n", j, wordsarr[j]);
		}
	}

	if(DEBUG){
		for(j = 0; j < arraylen; j++){
			printf("wordsarr[%d]: %s\n", j, wordsarr[j]);
		}
	}
	return wordsarr;
}
*/


void quickSort(char* wordsarr[], int arraylen, int left, int right)
{
	int i, j, pivot, k;
	char* x;
	char temp[arraylen];

	i = left;
	j = right;
	pivot = (left+right)/2;
	x = wordsarr[pivot];

	if(DEBUG)
	{
		printf("Unsorterd array: ");
		for(k=0; k<arraylen; k++)
		{
			printf("%s ", wordsarr[k]);
		}
		printf("\n");
	}

	do
	{
		while((strcmp(wordsarr[i],x) < 0) && (i < right))
		{
			i++;
		}
		while((strcmp(wordsarr[j],x) > 0) && (j > left))
		{
			j--;
		}
		if(i <= j)
		{
			strcpy(temp, wordsarr[i]); //swap
			strcpy(wordsarr[i], wordsarr[j]);
			strcpy(wordsarr[j], temp);
			i++;
			j--;
		}
	} while(i <= j);

	if(left < j)
	{
		quickSort(wordsarr, arraylen, left, j);
	}
	if(i< right)
	{
		quickSort(wordsarr, arraylen, i, right);
	}

	if(DEBUG)
	{
		printf("Sorterd array:\n");
		for(k = 0; k < arraylen; k++)
		{
			printf("%s\n", wordsarr[k]);
		}
		printf("\n");
	}


}

int main(int argc, char *argv[]) {
	/*
	   types of error:
           1. null string
           2. no input
           3. more than 1 input
           4. no alphabets
         */

	//Variable declarations.
	char *string = '\0';
	char *duplicatedstr;
	char *triplicatedstr;
	int isChar = 0;
	int notChar = 0;
	int i = 0;
	int len = 0;
	int numwords = 0;
	int arraylen = 0;

	//Can we please modularize this into an arg_check function?
  	if(argc != 2)
	{
		fprintf(stderr, "%s\n", "ERROR: Invalid number of arguments.");
    		exit(0);
  	}

	string = argv[1];
	if(strlen(string) == 0){
		fprintf(stderr, "%s\n", "String empty.");
		exit(0);
	}



	string = trimwhitespace(string);
	len = strlen(string);
	if(DEBUG){

		printf("String: %s\n", string);
		printf("String length: %d\n", len);
	}
	if(len == 0){

		fprintf(stderr, "%s\n", "ERROR: String empty.");
		exit(0);
	}
	if(DEBUG){
		printf("string: \"%s\"\n", string);
	}

	//Can the character check be modularized?
	
  	for(i = 0; i < strlen(string); i++)
  	{
		if((string[i] >= 'a' && string[i] <= 'z') || (string[i] >= 'A' && string[i] <= 'Z'))
		{
			isChar++;
    		}
    		else
    		{
      			notChar++;
    		}
  	}

	printf("isChar count: %d\n", isChar);
  	printf("notChar count: %d\n", notChar);

  	if(isChar == 0)
  	{
    		fprintf(stderr, "%s\n", "No alphabets");
    		exit(0);
	}




	/*
	  					--------------- Logic---------------

	  So we have one long string. We've determined which characters are alphabets, and which are non-alphabetic characters.

	  "thing stuff other stuff blarp"

	  # of words = 5
	  # of non-alphabetic characters = 4

	  Hence [since we already have the # of non-alphabetic characters] :

	  total # of words = (# of non-alphabetical characters) + 1

	  NOTE: THIS MATHEMATICAL RELATION ONLY HOLDS WHEN THERE IS EXACTLY ONE SEPARATOR BETWEEN EACH WORD. 

	  HOWEVER, THE ASSIGNMENT DOES NOT SAY THAT THERE WILL BE ONLY ONE SEPARATOR BETWEEN EACH WORD.

	  ASSUME NOTHING.

	  Take 2
	  -------

	  Dealing with multiple separators, et al. : overwrite each non-alphabetic character with a space.
	  Then tokenize string with strtok and pass space as a delimiter.
	  Voila! We should now have each [distinct] word in the string.

	  Determining the size of the array needed: iterate through the tokens.



	  Now that we have the # of words, we have the [required] size of the array required to store the words, and can thus determine
	  how much memory to allot to the array.

	  Now, we must build each word.

	  We traverse the string, and until we hit a non-alphabetic character, each character encountered is appended to "word" variable,
	  thus building up each word. Once a non-alphabetic character is encountered, the word has been built, so we dump it in the array,
	  and reset the word variable to "". Repeat this process until you reach the end of the string. At the end of the string, the end-
	  of-string character ('\0') acts as the final stopping point. That is, the last word has been built when the end-of-string
	  character is encountered.

	  At this point, the array has been populated with each word.

	  [Sorting logic will come here.]

	  					  ---------- End Logic ----------

	 */

	//overwrite all non-alphabetic characters with spaces

	
	for(char *p = string; *p; p++){
		if(!isalpha(*p)){
			*p = ' ';
		}
	}

	if(DEBUG){
		printf("String after overwriting non-alphabetic characters with spaces: \"%s\"\n", string);
	}

	duplicatedstr = strdup(string);

	char *tokens = strtok(string, " ");
	i = 0;
	while(tokens){
		i++;
		tokens = strtok(NULL, " ");
	}

	//tokens = strtok(string, " ");
	/*
	if(DEBUG){
		printf("0.Printing tokens...\n");
		while(tokens){
			printf("Token: %s\n", tokens);
			tokens = strtok(NULL, " ");
		}
	}
	*/

	arraylen = i;
	if(DEBUG){
		printf("arraylen: %d\n", arraylen);
	}

	char *wordsarr[arraylen];
	wordsarr[arraylen] = '\0';
	if(DEBUG){
		printf("Array created.\n");
	}

	tokens = strtok(duplicatedstr, " ");
	/*
	if(DEBUG){
		printf("Printing tokens..\n");
		while(tokens){
			printf("Token: %s\n", tokens);
			tokens = strtok(NULL, " ");
		}
	}
	*/
	
	//build_words(wordsarr, string, len, arraylen);
	i = 0;
	while(i < arraylen && tokens){
		wordsarr[i] = tokens;
		tokens = strtok(NULL, " ");
		i++;
	}

	if(DEBUG){
		for(i = 0; i < arraylen; i++){
			printf("wordarr[%d]: %s\n", i, wordsarr[i]);
		}
	}

		
	quickSort(wordsarr, arraylen, 0, arraylen-1);
  	return 0;
}

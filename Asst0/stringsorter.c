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
#include "stringsorter.h"

unsigned int DEBUG = 1; // NO DEBUG = 0 ; DEBUG = 1


char** buildwords( /* Parameters TBA */){

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
	int isChar = 0;
	int notChar = 0;
	int i = 0;
	int len = 0;
	int numwords = 0;
	//char *words[];
	int arraylen = 0;

	//Can we please modularize this into an arg_check function?
  	if(argc != 2)
	{
		fprintf(stderr, "%s\n", "ERROR: Invalid number of arguments.");
    		exit(0);
  	}

	string = argv[1];
	len = strlen(string);

	if(DEBUG){

		printf("String: %s\n", string);
		printf("String length: %d\n", len);
	}

	if(len == 0){

		fprintf(stderr, "%s\n", "ERROR: String empty.");
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

	numwords = notChar + 1;
	arraylen = numwords;
	char *words[] = malloc(arraylen*sizeof(char*));
	words[arraylen] = '\0';
	build_words();

  	return 0;
}

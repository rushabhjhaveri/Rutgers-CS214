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
	  char *end;

	    // Trim leading space
	    //while(isspace((unsigned char)*str)) str++;
	    while(!isalpha((unsigned char)*str)) str++;

	      if(*str == 0)  // All spaces?
		          return str;

	        // Trim trailing space
	        end = str + strlen(str) - 1;
		  //while(end > str && isspace((unsigned char)*end)) end--;
		  while(end > str && (!isalpha((unsigned char)*end))) end--;

		    // Write new null terminator
		    *(end+1) ='\0';

		      return str;
}

char** build_words(char *words[], char *string, int len, int arraylen){
	if(DEBUG){
		printf("Entered build_words \n");
	}
	int i = 0;
	int j = 0;
	int k = 0;
	char word[len];
	memset(word, '\0', sizeof(word));
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
			word[k] = ch;
			if(DEBUG){
				printf("Word: %s\n", word);
			}
			k++;
		}
		else{ //non-alphabetic character => word built
			//dump word in array
			if(DEBUG){
				printf("Word: %s\n", word);
			}
			storedword = (char*) malloc(strlen(word)+1);
		//	memset(storedword, '\0', sizeof(storedword));
			strcpy(storedword, word);
			words[j] = storedword;
		/*	if(i == (len-1)){
				words[j] = word;
			}*/
			if(DEBUG){
				printf("words[%d]: %s\n", j, words[j]);
			}
			memset(word, '\0', sizeof(word)) ;
			//word[0] = '\0';
			k = 0;
			j++;
		}
	} // end for

	if(isalpha(ch)){
		storedword = (char*) malloc(strlen(word)+1);
	//	memset(storedword, '\0', sizeof(storedword));
		strcpy(storedword, word);
		words[j] = storedword;
		if(DEBUG){
			printf("word[%d]: %s\n", j, words[j]);
		}
	}

	if(DEBUG){
		for(j = 0; j < arraylen; j++){
			printf("words[%d]: %s\n", j, words[j]);
		}
	}
	return words;
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
	int arraylen = 0;

	//Can we please modularize this into an arg_check function?
  	if(argc != 2)
	{
		fprintf(stderr, "%s\n", "ERROR: Invalid number of arguments.");
    		exit(0);
  	}

	string = argv[1];

	if(DEBUG){

		printf("String: %s\n", string);
		printf("String length: %d\n", len);
	}


	string = trimwhitespace(string);
	len = strlen(string);
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
	char *words[arraylen];
	words[arraylen] = '\0';
	if(DEBUG){
		printf("Array created.\n");
	}
	build_words(words, string, len, arraylen);

  	return 0;
}

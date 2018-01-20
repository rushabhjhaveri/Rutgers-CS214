//partner name: Rushabh Jhaveri
//partner netID: rrj28

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stringsorter.h"

int debug = 1;

int main(int argc, char *argv[]) {

  /*
  types of error:
  1. null string
  2. no input
  3. more than 1 input
  4. no alphabets
  */

  if(argc != 2)
  {
    fprintf(stderr, "%s\n", "ERROR: Invalid number of arguments");
    exit(0);
  }

  char * string = argv[1];
  printf("String: %s\n", string);

  int isChar = 0;
  int notChar = 0;
  int i;

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
  return 0;
}

#ifndef STRINGSORTER_H_ //include guard
#define STRINGSORTER_H_

char** build_words(char *wordsarr[], char *string, int len, int arraylen);
void quickSort(char* wordsarr[], int arraylen, int left, int right);

#endif

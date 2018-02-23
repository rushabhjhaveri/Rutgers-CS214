# File IO in C # 

In C, each file is a sequential stream of bytes. 

A file must first be opened properly before it can be accessed for reading and / or writing. When a file is opened, a __stream__ is associated with that file. 

Successfully opening a file returns a pointer to the address of the file, and null if it is unsuccessful. 

## File Structure ## 
__\*FILE__ : A structure containing the information about a file or text stream needed to perform input or output operations on it, including: 
* a file descriptor 
* the current stream position 
* an end-of-file indicator 
* an error indicator 
* a pointer to the stream's buffer, if applicable 

```
FILE * fp;
``` 
This declares a pointer variable of type __FILE__. It is assigned the address of file descriptor, i.e., an area of memory that will be associated with an input or output stream. 

Whenever you are to read from or write to the file, you must first open the file and assign the address of its file. 

## Opening Files ## 
Opening file for input [to read]: 
```
FILE * fp = fopen("<filename>", "r");
```
Opening files for output [to write]:
```
FILE * fp2 = fopen("<filename>", "w");
```
Once the files are open, they stay open until you close them or end the program. 

In general, fopen has the following syntax: 
```
FILE * fopen(const char *filename, const char *mode);
```
### Testing for Successful Open ### 
If the file was not able to be opened, fopen returns NULL. 

Thus, one can [should] always make a check if the file pointer after performing the fopen routine is NULL. If it is, it should be handled as seen fit. If it is not NULL, one can proceed as intended. 

## Predefined File Pointers in stdio.h ## 
__stdin__: A pointer to a FILE which refers to the standard input stream [usually the keyboard]. 

__stdout__: A pointer to a FILE which refers to the standard output stream [usually the display terminal]. 

__stderr__: A pointer to a FILE which refers to the standard error stream [usually the display terminal].  

## Standard IO ## 
When a program begins execution, three file streams are already defined and open - stdin, stdout, and stderr.  

stdin and stdout are set to "normal input / output", which is usually the keyboard and screen respectively. 

By default, stdin and stdout are buffered. This means it minimizes expensive system calls by sending data in chunks. The buffering can be controlled via the standard function __setbuf()__. 

The stderr stream is reserved for sending error messages. It is [typically] directed towards the screen, and is unbuffered. 

## File Manipulation and IO Functions ## 
__fopen()__: Opens a file for certain types of reading and writing. Returns a FILE pointer. 

__fclose()__: Closes a file associated with the FILE * value passed to it. 

__rewind()__: Positions the pointer to beginning of file. 

__fseek()__: Positions the pointer to any location within the file. 

__feof()__: Checks if end-of-file indicator has been set. 

__ferror()__: Checks whether the error indicator has been set for a given stream. 

## fopen Mode Parameter ## 
Mode: r, rb
Description: Open for reading. 
Starts: at the beginning of the file. 

Mode: w, wb 
Description: Open for writing [create file if it doesn't exist already]. Deletes content and overwrites the file. 
Starts: at the beginning of the file. 

Mode: a, ab 
Description: Open for appending [creates file if it doesn't exist]. 
Starts: at the end of the file. 

Mode: r+, rb+, r+b 
Dezcription: Open for reading and writing. 
Starts: at the beginning of the file. 

Mode: w+, wb+, w+b 
Description: Open for reading and righting. Deletes content and overwrites the file. 
Starts: at the beginning of the file. 

Mode: a+, ab+, a+b 
Description: Open for reading and writing [append if file exists]. 
Starts: at the end of the file. 

## fclose() ## 
To choose a file, pass the file pointer to fclose(). 
Definition:
```
int fclose(FILE * fp)
```
fclose() breaks the connection with the file and frees the file pointer. 

It is a good practice to free the file pointers when a file is no longer needed, because most OS's have a limit as to the number of files a program may have open at any given time. 

fclose() is called automatically for each open file when the program terminates. 

## Sequential File Operations ## 
Once a file is open, operations on the file [reading and writing] usually work through the file sequentially - from the beginning to the end. 

There are four basic types of file IO: 
* Character by character 
* Line by line 
* Formatted IO 
* Binary IO 

## Character Input ## 
Character input functions:
__fgetc()__: Returns one character from a file. 
__fgets()__: Gets a string from the file [ending at newline or EOF]. 
__fscanf()__: Works like the regular scanf() function. 

Return values: 
On success: the next character in the input stream. 
On error: EOF 
On end-of-file: EOF 

If the return value is EOF, one can determine what caused it by calling either __feof()__ or __ferror()__. 

## Character Output ## 
Character output functions: 
__fputc__: Writes one character to a file. 
__fputs()__: Writes a string to a file. 
__fprintf__: Enables printf() output to be written to any file. 

Note: __putchar(c)__ is equivalent to __putc(c, stdout)__. 

Return values: 
On success: the character that was written. 
On error: EOF. 

## Formatted IO ## 
```
int fprintf(FILE *fp, const char *format, ...);  
int fscanf(FILE *fp, const char *format, ...);
```
These functions are generalizations of printf() and scanf() respectively. 

## Line [String] Input ## 
Reading a line from a file: 
```
char *fgets(char *buf, int max, FILE *fp);
```
Returns after one of the following: 
* Reads (at most) max-1 characters from the file.  
* Reads a \n character.  
* Reaches end-of-file.  
* Encounters an error.   

Return values: 
* On success: pointer to buf. Note, fgets() automatically appends a \0
to the end of the string.
* On end-of-file: NULL. 
* On error: NULL. 

Use feof() or ferror() to determine if an error has occurred. 

## Line [String] Output ## 
Character strings may be written to file using 
```
int fputs(const char *str, FILE *fp);
```
Not actually line output. It does not automatically append a \n and consecutive calls may print strings on the same line. 

Return values:
On success: zero.
On error: EOF.

## Binary IO ## 
When reading and writing binary files, may deal with objects directly without first converting them to character strings.  
Direct binary IO provided by
```
size_t fread(void *ptr, size_t size, size_t nobj, FILE *fp);
size_t fwrite(const void *ptr, size_t size, size_t nobj, FILE *fp);
```
Can pass objects of any type. 

## Random File Operations ## 
IO is not confined to sequential motion through a file. May also shift the file position back and forth to any specified location. 

```
long ftell(FILE *fp);
int fseek(FILE *fp, long offset, int from);
void rewind(FILE *fp);
```
Operate differently on text files as to binary files. 

ftell() tells you where you are in the file. 

fseek() moves the file pointer associated with the stream to a new location that is offset bytes from the origin. 

Argument meaning:
* stream_pointer is a pointer to the stream FILE structure
of which the position indicator should be changed;
* offset is a long integer which specifies the number of
bytes from origin where the position indicator should be
placed;
* origin is an integer which specifies the origin position.
  It can be:
  * SEEK_SET: origin is the start of the stream
  * SEEK_CUR: origin is the current position
  * SEEK_END: origin is the end of the stream 
  

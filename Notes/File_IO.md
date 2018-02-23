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

__rewind()__: 

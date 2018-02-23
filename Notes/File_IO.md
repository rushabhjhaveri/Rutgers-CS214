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

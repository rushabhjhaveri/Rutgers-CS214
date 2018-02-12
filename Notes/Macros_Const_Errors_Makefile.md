macros
	- very powerful

#include -> preprocessor directive

const 
const int x = 5
OR
int const x = 5

const char * s;

const variables / pointers => immutable

Avoiding Errors
check argc before using argv

when using malloc make sure you check if malloc failed or not (check if null)

In Unix - two different output streams: stderr and stdoutput

Separate Compilation

header file: contains public interface of your library. Includes function prototypes, types, macros, constants

.c file: contains all non-public aspects. Includes function definitions, private definitions, 

Scope of functions in C:
accessed everywhere (default)
private (declared as static function)

extern variable - visible everywhere

static char memory[5000]
	- visible only within the file

What happens if the same header file gets included more than once?
Bad stuff happens.
Trick to prevent this: 

Stack
	- contains all local variables and parameters
	- fixed amount of memory
	- once a function's scope finishes, its variables etc get popped off the stack 
Heap
	- dynamic memory allocation
Global variables
Local variables

calloc - initializes all memory to zero

# Processes #

*Processes* are the primitive units for allocation of system resources. 

Each process has its own address space and [usually] one thread of control.  

A process executes a program; you can have multiple processes executing the same program, but each process has its own copy of the program within its own address space and executes it *independently* of the other copies.  

Processes are organized hierarchically.
Each process has a *parent process* which explicitly arranged to create it.  
The process created by a given parent are called its *child processes*.  
A child inherits many of its attributes from the parent process.  

## The fork() System Call ## 

The __fork()__ system call is used to create processes.  

It takes no arguments and returns a process ID.  

The purpose of __fork()__ is to create a *new* process, which becomes the *child* process of the caller.  

After a new child process is created, *both* processes will execute the next instruction following the __fork()__ system call.  

Thus, we need to distinguish between the parent and the child.  

This can be done by testing the returned value of __fork()__.  
* If __fork()__ returns a negative value, the creation of the child process was unsuccessful.  
* __fork()__ returns a zero to the newly created child process.  
* __fork()__ returns a positive value, the *process PID* of the child process, to the parent.   

The returned process ID is of type pid_t defined in sys/types.h.  

Normally, the process ID is an integer.  

A process can use function __getpid()__ to retrieve the process ID assigned to this process.  

__Note__: Unix will make an exact copy of the parent's address s[ace and give it to the child. Therefore, the parent and child processes have separate address spaces.  

If the call to __fork()__ is executed successfully, Unix will: 
* make two identical copies of address spaces; one for the parent, and one for the child.  
* Both processes will start their execution at the next statement following the __fork()__ call.  

Both processes start their execution right after the __fork()__ system call.  

Since both processes have identical but separate address spaces, those variables initialized __before__ the __fork()__ call have the same values in both address spaces.  

Since every process has its own address space, any modification will be independent of the others.  

In other words, if the parent changes the value of its variable, the modification will only affect the variable in the parent process's address space.  

Other address spaces created by __fork()__ calls will not be affected even though they have identical variable names.  

# Threads # 

Threads allow for spawning concurrent process flow. 

Most effective on multiprocessor or multicore systems where the process flow can be scheduled to run on another processor, thus gaining speed through *parallel* or *distributed* processing.

Threads require less overhead than forking or spawning a new process because the system does not initialize a new system virtual memory space and environment for the process.

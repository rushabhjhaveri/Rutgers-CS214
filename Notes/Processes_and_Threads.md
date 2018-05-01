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

All threads within a process share the same address space.  

A thread is spawned by defining function and it's arguments which will be processed in the thread. 

The purpose of using the POSIX thread library is to make software execute faster.  

## Thread Basics ## 

Thread operations include:  
* creation 
* termination 
* synchronization [joining, blocking] 
* scheduling 
* data management 
* process interaction 

A thread does not maintain a list of created threads, nor does it know the thread that created it.  

All threads within a process share the same address space. 

Threads within the same process share: 
* process instructions 
* *most* data 
* open files [descriptors] 
* signals and signal handlers 
* current working directory 
* user and group ID 

Each thread has a unique: 
* thread ID 
* set of registers, stack pointer 
* stack for local variables, return addresses 
* signal mask 
* priority 
* return value: errno 

pthread functions return 0 on success / if OK.  


## Thread Creation and Termination ## 

Example code: 

```
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function( void *ptr );

main()
{
	pthread_t thread1, thread2;
	char *message1 = "Thread 1";
	char *message2 = "Thread 2";
	int  iret1, iret2;

	/* Create independent threads each of which will execute function */

	iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
	iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);

	/* Wait till threads are complete before main continues. Unless we  */
	/* wait we run the risk of executing an exit which will terminate   */
	/* the process and all threads before the threads have completed.   */

	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL); 

	printf("Thread 1 returns: %d\n",iret1);
	printf("Thread 2 returns: %d\n",iret2);
	exit(0);
}

void *print_message_function( void *ptr )
{
	char *message;
	message = (char *) ptr;
	printf("%s \n", message);
}
```

__Compiling__ 

C compiler: gcc -lpthread pthread1.c

C++ compiler: g++ -lpthread pthread1.c 


In this example, the same function is used in each thread. 
The arguments are different. 
The functions need not be the same.  

Threads terminate by: 
* explicitly calling *pthread_exit()*
* letting the function return
* a call to the function *exit()* which will terminate the process, including any threads. 

### pthread_create() ###

```

int pthread_create(pthread_t * thread, 
		const pthread_attr_t * attr,
		void * (*start_routine)(void *), 
		void *arg);
```

thread: returns the thread id [unsigned long int defined in bits/pthreadtypes.h].  

attr: Set to NULL if default thread attributes are used, else define members of the struct pthread_attr_t defined in bits/pthreadtypes.h. Attributes include: 
* detached state [joinable? Default: PTHREAD_CREATE_JOINABLE; other option: PTHREAD_CREATE_DETACHED] 

* scheduling policy [real-time? PTHREAD_INHERIT_SCHED, PTHREAD_EXPLICIT_SCHED, SCHED_OTHER]  

* scheduling parameter  

* inheritsched attribute [default: PTHREAD_EXPLICIT_SCHED; Inherit from parent thread: PTHREAD_INHERIT_SCHED]  

* scope [kernel threads: PTHREAD_SCOPE_SYSTEM; User threads: PTHREAD_SCOPE_PROCESS; Pick one or the other; not both].  

* guard size 

* stack address [See unistd.h and bits/posix_opt.h_POSIX_THREAD_ATTR_STACKADDR]  

* stack size [default minimum PTHREAD_STACK_SIZE set in pthread.h]  

void * ( * start_routine): pointer to the function to be threaded; fuction has a single argument - pointer to void 

\* arg: pointer to argument of function to be threaded; to pass multiple arguments, send a pointer to a structure  


### pthread_exit() ### 

```
void pthread_exit(void * retval); 
```

retval: return value of thread  

This routine kills the thread.  

The pthread_exit() function never returns.  

If the thread is not detached, the thread id and return value may be examined from another thread by using pthread_join.  

The return pointer * retval must not be of local scope, otherwise, it would cease to exist once the thread terminates.  


## Thread Synchronization ## 



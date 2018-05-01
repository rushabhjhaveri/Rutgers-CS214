#include "libnetfiles.h"
//#include <string.h>
//#include<stdlib.h>
//#include<stdio.h>
//#include<sys/types.h>
#define PERMS_ERROR 100000
#define BUFFERSIZE 4096
#define PATHSIZE 256
#define DEBUG1 1

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{

    int fildes;
    ssize_t bytes_read;
    ssize_t bytes_write;
    int   fileMode1;
    int   filePerms;
    char *buffer;
    char pathName[PATHSIZE];
    int close_retval;

    char tmpstr;    // just to make the program pause

	if (argc < 6)
	{
		fprintf(stderr,"usage %s hostname pathname fileMode filePermissions(0=O_RDONLY/1=O_WRONLY/2=O_RDWR), <\"some Write string\"> \n", argv[0]);
		exit(0);
	}

        // call netserverinit   to check if the hostname exists
        // 2 argument is filemode 0 = UNRESTRICTED, 1 = EXCLUSIVE and 2 = TRANSACTION


        // get the fileMode from the command line

	fileMode1 = atoi(argv[3]);
        if (DEBUG1) {
           printf("filemode1 = [%d]\n",fileMode1);
        }
        if (netserverinit(argv[1],fileMode1) != 0) {
           error("hostname NOT FOUND");
        }




        // get the pathName from the command line
        memset(pathName, '\0', sizeof(pathName));  
        strcpy(pathName,argv[2]);

        // get the filePermissions from the command line
	filePerms = atoi(argv[4]);

        if (DEBUG1) {
           printf("filePerms = [%d]\n",filePerms);
	   printf("FileName with full path to open: \n");
        }


        
        // now test the call the netopen function at this stage
         fildes = netopen(pathName, filePerms );
	// int fildes2 = netopen("file2.txt", filePerms);

        if (DEBUG1) {
           printf("main - File descriptor returned by netopen is [%d]\n", fildes);
           printf(" main - errno = [%d] h_errno = [%d] \n", errno,h_errno);
        }
        if (fildes == -1) {
            if (errno == PERMS_ERROR) {
               printf("main - Cannot open files - Permissions error \n");
            } else {
                printf("main - ERROR: %s\n", strerror(errno) );
            }
           exit(1);
        }

        printf("==============================END OF OPEN====================\n");
        printf("Enter any character to continue.....\n");
         scanf("%c", &tmpstr);

	FILE * file = fopen(pathName, "r");
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	printf("\nSIZE: %d\n", size);

        // now test the call the netread function at this stage
	buffer = malloc( size);
	bzero(buffer,size);
        // bytes_read = netread(fildes, &buffer, BUFFERSIZE); 
        bytes_read = netread(fildes, buffer, size); 

        if (DEBUG1) {
           printf("main - Data receivd in buffer  [%s]\n", buffer);
           printf("main - Bytes Read returned by netread is [%ld]\n", bytes_read);
           printf("main - errno = [%d] h_errno = [%d] \n", errno,h_errno);
        }
        if (bytes_read < 0) {
            if (errno == PERMS_ERROR) {
               printf("main - Cannot open files - Permissions error \n");
            } else {
                printf("main - ERROR: %s\n", strerror(errno) );
            }
            // exit(1);
        }
        printf("==============================END OF READ====================\n");
	


	if (bytes_read >=0) {

		// now test the call the netwrite function at this stage
		bzero(buffer,size);
		bytes_write = 0;
		strcpy(buffer, argv[5]);   // 6 is the write string parameter
		strcat(buffer, "\n");
		if (DEBUG1) {
			printf("main - Data in buffer  before call the netwrite[%s]\n", buffer);
		}
		bytes_write = netwrite(fildes, buffer, strlen(buffer)); 
		if (DEBUG1) {
			printf("main - Data in buffer  after netwrite[%s]\n", buffer);
			printf("main - Bytes Write returned by netwrite is [%ld]\n", bytes_write);
			printf("main - errno = [%d] h_errno = [%d] \n", errno,h_errno);
		}
		if (bytes_write < 0) {
			if (errno == PERMS_ERROR) {
				printf("main - Cannot open files - Permissions error \n");
			} else {
				printf("main - ERROR: %s\n", strerror(errno) );
			}
			// exit(1);
		}

		printf("==============================END OF WRITE====================\n");
	}

        // now test the call the netwrite function at this stage
	bzero(buffer,size);


        close_retval = netclose(fildes);
//	int close2 = netclose(fildes2);	
	
        if (DEBUG1) {
           printf("main - close_retval returned by netclose is [%d]\n", close_retval);
//	printf("main - close2: [%d]\n", close2);
           printf("main - errno = [%d] h_errno = [%d] \n", errno,h_errno);
        }


	return 0;
}

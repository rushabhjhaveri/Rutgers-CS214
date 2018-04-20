#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>
#include<sys/socket.h>
#include<time.h>
#include<sys/time.h>
#include<netdb.h>
#include<sys/stat.h>
#include<signal.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<fcntl.h>
#include "netfileserver.h"

unsigned int debug = 1;

/*
   Adds a file to list clientlist. 

   Returns file descriptor on completion.

*/

int addFileToList(struct filelist * ptr, int clientfd, int mode, int flag){
	int pipestat; /* Holds value returned by pipe(int fd[2]). */
	
	if(debug){
		printf("ADDING TO LIST...\n");
		printf("parameters: clientfd: %d, mode: %d, flag: %d\n", clientfd, mode, flag)
	}
	
	pthread_mutex_lock(&list);
	
	/* Initially, prev and curr point to same node. */
	struct clientlist * curr = ptr->list;
	struct clientlist * prev = ptr->list;
	
	int fd = 0;
	
	/* If clientlist is empty, add at the beginning. */
	
	if(curr == 0){
		ptr->list = calloc(1, sizeof(clientlist));
		curr = ptr->list;
		curr->client = clientfd;
		gettimeofday(&curr->start, NULL);
		curr->mode = mode;
		curr->flag = flag;
		pipestat = pipe(curr->fd);
		if(pipestat == -1){
			printf("[%d] %s\n", errno, strerror(errno));
		}
		fd = (curr->fd)[0];
		pthread_mutex_unlock(&list);
		
		if(debug){
			printf("In addFiles, printing struct when curr is empty:\n");
			printf("In addFiles, curr->client: %d, curr->mode: %d, curr->flag: %d", curr->client, curr->mode, curr->flag);
			printf("curr empty, fd: %d\n", fd);
		}
		return fd;
	}
	
	/* If clientlist is nonempty, add at the end. */
	
	while(curr != NULL){
		prev = curr;
		curr = curr->next;
	}
	
	prev->next = calloc(1, sizeof(clientlist));
	prev = prev->next;
	prev->client = clientfd;
	gettimeofday(&prev->start, NULL);
	prev->mode = mode;
	prev->flag = flag;
	
	pipestat = pipe(prev->fd);
	
	if(pipestat == -1){
		printf("[%d] %s\n", errno, strerror(errno));
	}
	
	fd = (prev->fd)[0];
	
	pthread_mutex_unlock(&list);
	
	if(debug){
			printf("In addFiles, printing struct:\n");
			printf("In addFiles, prev->client: %d, prev->mode: %d, prev->flag: %d", prev->client, prev->mode, prev->flag);
			printf("return fd: %d\n", fd);
		}
	return fd;
}

/*
   Given a node, find the nnode that contains node.

   If not found, add file to end of filelist.

   Returns filelist structure.

*/

struct filelist * lookup_file(char * filename, int node){
	
	/* Initially, curr and prev point to the same node. */
	if(debug){
		printf("0. In lookup_files, paramteres: filename: %s, node: %d\n", filename, node);
	}
	
	struct filelist * curr = fL;
	struct filelist * prev = curr;
	
	pthread_mutex_lock(&list);
	
	/* Loop through filelist looking for a match. */
	while(curr != NULL){

		/* If a match is found, return corrosponding filelist struct. */
		if(curr->nnode == node){
			pthread_mutex_unlock(&list);
			return curr;
		}

		prev = curr;
	}
	
	/* If not found, add to end of filelist. */
	prev->next = calloc(1, sizeof(filelist));
	prev = prev->next;
	prev->filename = filename;
	prev->nnode = node;
	pthread_mutex_unlock(&list);
	
	if(debug){
		printf("1. In look_up files, printing struct:\n");
		printf("In addFiles, prev->filename: %d, prev->nnode: %d\n", prev->filename, prev->nnode);

	}
	
	return prev; /* Return newly added node. */
}

/*
   Checks if the file exists and is able to be opened.
   
   stat(char * path, struct stat *buf) 
   	- stats the file pointed to by path and populated buf.
	- On successm zero is returned.
	- On error, -1 is returned, and errno is set appropriately.
*/

int is_file(const char * path){
	struct stat filepath;
	int status;
	status = stat(path, &filepath);
	
	/* If stat() failed. */
	if(status == -1){
		/* Print out error message. */
		printf("[%d] %s\n", errno, strerror(errno)); /* Added this. */
		return status;
	}
	else{ /* stat() was successful. */
		/* Check if given path is a directory. */
		if(S_ISDIR(filepath.st_mode)){
			errno = EISDIR; /* Set errno appropriately. */
			return 0;
		}
	}
	
	return 1;
}

int rdwr_conflict(char * filename, int mode, int flag, int clientfd, int rd){

	return 0;
}

/*
   Attempts to open a file.


*/
int openFile(int clientfd){

	char * path = calloc(1, 256);
	int flags;
	int mode;
	int error;
	int conflict_mode;
	int retfd;
	int recmode;
	int recflags;
	int recpath;
	int temp;

	/* Receives connection mode. */
	recmode = 0;
	while(recmode < sizeof(int)){
		recmode += recv(clientfd, &mode, sizeof(int), 0);
		if(recmode == 0){
			continue;
		}
	}
	
	/* Receives open flags [RDONLY, WRONLY, RDWR]. */
	recflags = 0;
	while(recflags < sizeof(int)){
		recflags += recv(clientfd, &flags, sizeof(int), 0);
		if(recflags == 0){
			continue;
		}
	}
	
	/* Receives filepath. */
	recpath = 0;
	while(recflags < 256){
		temp = 0;
		temp = recv(clientfd, path, 256, 0);
		recflags += temp;
		if(temp == 0){
			continue;
		}
	}
	
	if(debug){
		printf("0. In openFile, recmode: %d, recflags: %d, recpath: %d\n", recmode, recflags, recpath);
	}

	/* 
	   Checks if file is a regular file.

	   Checks for client mode. 
	*/
	if(is_file(path)){
		errno = 0;
		pthread_mutex_lock(&conflict);
		conflict_mode = rdwr_conflict(path, mode, flags, clientfd, 0);
		pthread_mutex_unlock(&conflict);
		
		if(errno){
			error = errno;
			printf("[%d] %s\n", errno, strerror(errno));
			retfd = -1;
		}
		if(conflict_mode > 0 && errno == 0){
			while(1){
				int isconflict = 0;
				if(debug){
					printf("1. In openFile, extensionA loop, conflict: %d\n", conflict_mode);
				}
				read(conflict_mode, &isconflict, sizeof(isconflict));
				if(isconflict == -1){
					//printf("TIMEOUT\n");
					retfd = -1;
					errno = EWOULDBLOCK;
					send(clientfd, &retfd, sizeof(retfd), 0);
					send(clientfd, &errno, sizeof(errno),0);
					return errno;
				}

				if(debug){
					printf("isconflict: %d\n", isconflict);
				}
				pthread_mutex_lock(&conflict2);
				conflict_mode = rdwr_conflict(path, mode, flags, clientfd, 0);
				if(debug){
					printf("isconflict = %d\n", isconflict);
				}
				pthread_mutex_unlock(&conflict2);

				if(conflict_mode <= 0){
					retfd = conflict_mode;
					break;
				}
			} // while(1) loop
		} // if(conflict > 0 && errno == 0) loop
		else {
			if(retfd == -1){
				error = errno;
			}
			else {
				retfd = conflict_mode;
			}
		}
	} //if_file
	else {
		retfd = -1;
		error = errno;
	}


	if(debug){
		printf("2. In openFlags, flags: %d, mode: %d, filename: %s\n", flags, mode, path); 
	}

	int retnbytes = send(clientfd, &retfd, sizeof(retfd), 0);
	int retnerror = send(clientfd, &error, sizeof(error), 0);
	if(debug){
		printf("3. In openFlags, retnbytes: %d, retnerror: %d\n", retnbytes, retnerror);
	}
	if(retnbytes == -1 || retnerror == -1){
		printf("ERROR: Send not successful\n");
		return -1;
	}

	if(debug){
		printf("4. In openFlags, retfd: %d, error: %d\n", retfd, error);
	}

	return retfd;
}

int main(int argc, char * argv[]){return 0;}

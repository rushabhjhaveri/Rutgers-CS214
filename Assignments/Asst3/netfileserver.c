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


void * readFileHelper2(void * sockList){
	
	int sd;
	int status;
	int isaccepted;
	int error;
	int reader;
	int retError;
	int readBytes;
	int retBytes;
	size_t nbytes;
	char * buffer;
	char ipstr[INET6_ADDRSTRLEN];
	socklen_t addrLen;
	struct socketlist * sL = (struct socketlist *) sockList;
	struct sockaddr_in serv_addr = sL->server;

	addrLen = sL->serverLen;
	sd = sL->client;
	status = listen(sd, MAX_CLIENTS);

	if(status == -1){
		if(debug){
			printf("Could not listen.\n");
		}
		return 0;
	}

	isaccepted = accept(sd, (struct sockaddr *) &serv_addr, &addrLen);

	inet_ntop(AF_INET, &serv_addr.sin_addr, ipstr, sizeof(ipstr));
	
	if(isaccepted == -1){
		printf("[%d] %s\n", h_errno, strerror(h_errno));
		return 0;
	}

	nbytes = sL->nbytes;
	buffer = sL->buffer;
	error = 0;
	reader = sL->nread;

	retError = send(isaccepted, &error, sizeof(error), 0);
	if(reader == -1 || retError == -1){
		free(buffer);
		return 0;
	}

	readBytes = send(isaccepted, &reader, sizeof(reader), 0);
	if(readBytes == -1){
		free(buffer);
		if(debug){
			printf("Sending error.\n");
		}
		return 0;
	}

	retBytes = send(isaccepted, buffer, nbytes, 0);
	if(retBytes == -1){
		free(buffer);
		if(debug){
			printf("Sending error.\n");
		}
		return 0;
	}

	if(debug){
		printf("In readFileHelper2: sent [%d] bytes.\n", retBytes);
	}
	free(buffer);
	return 0;
}

/*
   Helper method for readFile().

   Handles reads where the number of bytes to be read is > 4096 [Extension B].
*/
void readFileHelper(int client, int fd, size_t nbytes){
	int i;
	int bindable;
	int portsArr[MAX_SOCKETS];
	int isFirst;
	int sendPorts;
	size_t segment;
	char * buffer;
	pthread_t threadid[MAX_SOCKETS];

	/* Initialize portsArr. */
	bzero(portsArr, sizeof(portsArr));

	struct socketlist * sL[MAX_SOCKETS];
	bindable = 0;

	pthread_mutex_lock(&sockets);

	if(debug){
		printf("In readFileHelper, Ports: ");
	}

	for(i = 0; i < MAX_SOCKETS; i++){
		if(bindable == MAX_STREAMS_PER_CLIENT){
			break;
		}
		if(socketL[i].clientsocket == 0){
			socketL[i].clientsocket = client;
			socketL[i].part = bindable;
			socketL[i].fd = fd;
			sL[bindable] = &socketL[i];
			portsArr[bindable] = socketL[i].port;
			bindable++;
			if(debug){
				printf("%d ", socketL[i].port);
			}
		}
	}

	if(debug){
		printf("\n");
	}

	pthread_mutex_unlock(&sockets);

	if(bindable == 0){
		portsArr[0] = (-1) * ECONNRESET;
		send(client, &portsArr, sizeof(portsArr), 0);
		if(debug){
			printf("No more sockets can be bound.\n");
		}
		return;
	}

	errno = 0;
	isFirst = 1;
	if(debug){
		printf("In readFileHelper: bindable: [%d]\n", bindable);
	}

	for(i = 0; i < bindable; i++){
		segment = nbytes/bindable;
		if(isFirst){
			segment += nbytes % bindable;
			isFirst = 0;
		}
		buffer = calloc(1, segment);
		sL[i]->nread = read(fd, buffer, segment);
		sL[i]->buffer = buffer;
		sL[i]->nbytes = segment;
	}

	if(errno != 0){
		printf("[%d] %s\n", errno, strerror(errno));
		pthread_mutex_lock(&sockets);
		for(i = 0; i < bindable; i++){
			sL[i]->clientsocket = 0;
		}
		pthread_mutex_unlock(&sockets);
		portsArr[0] = errno * (-1);
		send(client, &portsArr, sizeof(portsArr), 0);
		return;
	}

	sendPorts = send(client, &portsArr, sizeof(portsArr), 0);

	if(sendPorts == -1){
		if(debug){
			printf("Error sending ports.\n");
		}
		return;
	}

	for(i = 0; i < bindable; i++){
		pthread_create(&threadid[i], NULL, readFileHelper2, (void *) sL[i]);
	}

	for(i = 0; i < bindable; i++){
		pthread_join(threadid[i], 0);
	}

	pthread_mutex_lock(&sockets);
	for(i = 0; i < bindable; i++){
		sL[i]->clientsocket = 0;
	}

	pthread_mutex_unlock(&sockets);
	
	return;
}


/*
   netread() handler on the server-side.
*/
int readFile(int client){
	int fd;
	int error;
	int reader;
	int recfd;
	int recNByte;
	int sendError;
	int sendReadBytes;
	int sendNBytes;
	size_t nbyte;
	char * buffer;
	
	/* Receive filedescriptor. */
	recfd = 0;
	while(recfd < sizeof(int)){
		recfd += recv(client, &fd, sizeof(int), 0);
		if(recfd == 0){
			continue;
		}
	}

	/* Receive number of bytes to be read. */
	recNByte = 0;
	while(recNByte < sizeof(size_t)){
		recNByte += recv(client, &nbyte, sizeof(size_t), 0);
		if(recNByte == 0){
			continue;
		}
	}

	/* If # of bytes > 4096 - Extension B. Use threads.*/
	if(nbyte > 4096){
		if(fd % 5 == 0){
			fd /= (-5);
		}
		else{
			reader = -1;
			if(debug){
				printf("Handle later.\n");
			}
		}

		readFileHelper(client, fd, nbyte);
		if(debug){
			printf("Done reading client [%d]\n", client);
		}
		return 0;
	}

	/* Allocate memory for the buffer. */
	buffer = calloc(1, nbyte);

	if(fd % 5 == 0){
		fd /= (-5);
		reader = read(fd, buffer, nbyte);
		if(debug){
			printf("Read bytes: [%d]\n", reader);
		}
	}
	else{
		reader = -1;
	}

	if(reader == -1){
		error = errno;
		if(error == 0){
			error = -1;
		}
	}
	else{
		error = 0;
	}

	/* Send error. */
	sendError = send(client, &error, sizeof(error), 0);

	if(reader == -1){
		free(buffer);
		return -1;
	}

	/* Send number of bytes [actually] read. */
	sendReadBytes = send(client, &reader, sizeof(reader), 0);

	/* Send populated buffer. */
	sendNBytes = send(client, buffer, nbyte, 0);

	/* Check if any of the sends failed. Free buffer, return -1. */
	if(sendError == -1 || sendReadBytes == -1 || sendNBytes == -1){
		free(buffer);
		return -1;
	}

	if(debug){
		printf("Sent: [%d] bytes.\n", sendNBytes);
	}

	free(buffer);

	return 0;
}


/*
   Adds a file to list clientlist. 

   Returns file descriptor on completion.

*/

int addFileToList(struct filelist * ptr, int clientfd, int mode, int flag){
	int pipestat; /* Holds value returned by pipe(int fd[2]). */
	
	if(debug){
		printf("ADDING TO LIST...\n");
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
	
	return fd;
}

/*
   Given a node, find the nnode that contains node.

   If not found, add file to end of filelist.

   Returns filelist structure.

*/

struct filelist * lookup_file(char * filename, int node){
	
	/* Initially, curr and prev point to the same node. */
	
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

/*
   For extension A.

   Checks for transfer mode conflicts, and also updates linked list.
*/

int rdwr_conflict(char * filename, int mode, int flag, int clientfd, int rd){
	struct stat filestatus;
	struct fdlist * filedescrips = fdL;
	struct fdlist * candidate = 0;
	struct filelist * curr = 0;
	
	int tempmode = 0;
	int tempmode2 = 0;
	int wr = 0; /* denotes write mode. */
	int nnode;
	int fd = 0; /* file descriptor returned by this function. */
	int openfd; /* file descriptor returned on calling open() system call. */
	
	stat(filename, &filestatus); 
	
	/* st_ino is the file serial number. Distinguishes this file from all other files on the same device. */
	nnode = filestatus.st_ino; 

	pthread_mutex_lock(&lock);
	if(fL == 0){
		if(debug){
			printf("In rdwr_conflict, Adding file to linked list.\n");
		}
		fL = calloc(1, sizeof(filelist));
		fL->filename = filename;
		fL->nnode = nnode;
	}

	curr = lookup_file(filename, nnode);
	
	/* Make variable assignments based on conditions. */
	while(filedescrips != 0){
		if(filedescrips->next == 0){
			candidate = filedescrips;
		}

		if(filedescrips->nnode == nnode && filedescrips->mode == 0){
			wr = 0;
			tempmode = 1;
			continue;
		}

		if(filedescrips->nnode == nnode && filedescrips->mode == 1 && filedescrips->filemode == O_RDONLY){
			tempmode2 = 1;
			continue;
		}

		if(filedescrips->nnode == nnode && filedescrips->mode == 1 && filedescrips->filemode != O_RDONLY){
			wr = 1;
			tempmode = 1;
			continue;
		}

		if(filedescrips->nnode == nnode && filedescrips->mode == 2){
			if(rd == 0){
				fd = addFileToList(curr, clientfd, mode, flag);
				pthread_mutex_unlock(&lock);
				return fd;
			}

			pthread_mutex_unlock(&lock);
			return -1;
		}

		filedescrips = filedescrips->next;
	}

	if((tempmode || tempmode2) && mode == 2){
		if(rd == 0){
			fd = addFileToList(curr, clientfd, mode, flag);
			pthread_mutex_unlock(&lock);
			return fd;
		}

		pthread_mutex_unlock(&lock);
		return -1;
	}

	filedescrips = candidate;

	if(rd == 0){
		if((tempmode == 0) || (tempmode2 == 1 && wr == 1 && flag == O_RDONLY) || (tempmode2 == 1 && wr == 0) || filedescrips == 0){
			errno = 0;
			openfd = open(filename, flag) * (-5);
			if(errno != 0 || openfd == -1){
				pthread_mutex_unlock(&lock);
				return -1;
			}

			if(filedescrips == 0){
				fdL = calloc(1, sizeof(fdlist));
				candidate = fdL;
			}
			else{
				candidate->next = calloc(1, sizeof(filelist));
				candidate = candidate->next;
			}

			candidate->filename = filename;
			candidate->nnode = nnode;
			candidate->mode = mode;
			candidate->filemode = flag;
			pthread_mutex_unlock(&lock);
			candidate->fd = openfd;
			return candidate->fd;
		}
		else{
			fd = addFileToList(curr, clientfd, mode, flag);
			pthread_mutex_unlock(&lock);
			return fd;
		}
	}
	else{
		if((tempmode2 == 0) || (tempmode2 == 1 && wr == 1 && flag == O_RDONLY) || (tempmode2 == 1 && wr == 0) || filedescrips == 0){
			pthread_mutex_unlock(&lock);
			return 0;
		}
		else{
			pthread_mutex_unlock(&lock);
			return -1;
		}
	}
	
	pthread_mutex_unlock(&lock);
	return -1;
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

/*
   This is where the thread begins. 

   Receives packet which is an int corrosponding to desired mode.
*/
void * start(void * c){
	int client = *(int *) c;
	int t;
	int cmd = 0;
	while(cmd < sizeof(t)){
		cmd += recv(client, &t, sizeof(t), 0);
		if(cmd == 0){
			continue;
		}
	}
	if(debug){
		printf("In start, Request type: [%d]\n", t);
	}
	switch(t){
		case 1: if(debug){printf("in case 1\n");}
			openFile(client);
			break;
		case 2: if(debug){printf("in case 2\n");}
			readFile(client);
			break;
	}

	close(client);
	free(c);
	return 0;
}

int main(){
	
	signal(SIGPIPE, SIG_IGN);
	pthread_t threadid;
	int sockfd;
	int bind_socket;
	int status;
	int client;
	int * c;
	char ipstr[INET6_ADDRSTRLEN];
	struct sockaddr_in serv_addr;
	
	/* Create socket. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/*Exit upon socket creation failure. */
	if(sockfd == -1){
		fprintf(stderr, "ERROR: Could not open socket, please check connection.\n");
		exit(1);
	}

	/* Make socket struct assignments. */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addrLen = sizeof(serv_addr);
	
	/* Bind sockaddr struct to socket. */
	bind_socket = bind(sockfd, (struct sockaddr *) &serv_addr, addrLen);

	/*Exit if bind fails. */
	if(bind_socket == -1){
		fprintf(stderr, "ERROR: Could not bind to socket.\n");
		exit(0);
	}

	/* Initialize the mutex. */
	if(pthread_mutex_init(&lock, NULL) != 0 || pthread_mutex_init(&list, NULL) != 0 || pthread_mutex_init(&sockets, NULL) != 0 || pthread_mutex_init(&conflict, NULL) != 0 || pthread_mutex_init(&conflict2, NULL) != 0){
		fprintf(stderr, "ERROR: Mutex init failed.\n");
		return 1;
	}
	
	/* Initialize errno. */
	errno = 0;
	
	/* Start listening. */
	status = listen(sockfd, MAX_CLIENTS);

	/* Return 0 if listen() fails. */
	if(status == -1){
		fprintf(stderr, "Listener error.\n");
		return 0;
	}

	printf("Listening on port [%d]\n", PORT);
	
	/* Wait indefinitely until a connection is accepted. */
	while(1){
		client = accept(sockfd, (struct sockaddr *)&serv_addr, &addrLen);
		c = (int *)calloc(1, sizeof(int));
		*c = client;
		
		/* Client info. */
		inet_ntop(AF_INET, &serv_addr.sin_addr, ipstr, sizeof(ipstr));
		printf("Connection established with %s\n", ipstr);
		pthread_create(&threadid, NULL, start, (void *)c);
	}
			
	return 0;
}

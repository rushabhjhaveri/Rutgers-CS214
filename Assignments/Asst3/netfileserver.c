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






fileModePerms  mapFileModePerms(int fileMode, int flags) {

	fileModePerms retVal;

	switch(fileMode) {
		case UNRESTRICTED:
			switch (flags) {
				case O_RDONLY:
					retVal=U_R;
					break;
				case O_WRONLY:
					retVal=U_W;
					break;
				case O_RDWR:
					retVal=U_RW;
					break;
			}
			break;
		case EXCLUSIVE:
			switch (flags) {
				case O_RDONLY:
					retVal=E_R;
					break;
				case O_WRONLY:
					retVal=E_W;
					break;
				case O_RDWR:
					retVal=E_RW;
					break;
			}
			break;
		case TRANSACTION:
			switch (flags) {
				case O_RDONLY:
					retVal=T_R;
					break;
				case O_WRONLY:
					retVal=T_W;
					break;
				case O_RDWR:
					retVal=T_RW;
					break;
			}
			break;
	}
	
	return retVal;
}

int isFileModePermsOKtoOpen(fileModePerms fmp1, fileModePerms fmp2) {
	if(debug){
		printf("in fileModePerms, fmp1:[%d], fmp2:[%d]\n", fmp1, fmp2);
	}
	if ( ((fmp1 == U_R) || (fmp1 == E_R)) && (fmp2 <=5) ) {
		// U_R is unrestricted_read
		// E_R is exclusive_read
		if (debug) {
			printf("isFileModePermsOK: [%d][%d] -- OK\n",fmp1, fmp2);
		}
		return(1);

	} else  if ( ((fmp1 == U_W) || (fmp1 == U_RW)) && (fmp2 <=3) ) {
		// U_W is unrestricted_write
		// E_R is unrestricted_readwrite
		if (debug) {
			printf("isFileModePermsOK: [%d][%d] -- OK2\n",fmp1, fmp2);
		}
		return(1);

	} else  if ( ((fmp1 == E_W) || (fmp1 == E_RW)) &&
			((fmp2 ==U_R) || (fmp2 ==E_R))
		   ) {
		// E_W is exclusive_write
		// E_RW is exclusive_readwrite
		if (debug) {
			printf("isFileModePermsOK: [%d][%d] -- OK3\n",fmp1, fmp2);
		}
		return(1);
	} else  {
		if (debug) {
			printf("isFileModePermsOK: [%d][%d] -- NOT OK\n",fmp1, fmp2);
		}
		return(0);
	}

	return 0;
}

// this will get a socket for a thread to use for Ext B
int getclients(struct socketlist * s, int port){
	int sd;
	// init server addr and client addr
	struct sockaddr_in serv_addr;
	// Attempt to create socket
	sd = socket(AF_INET, SOCK_STREAM, 0);
	// If socket creation fails exit
	if(sd < 0){
		printf("ERROR: Could not open socket, please check connection!\n");
		return -1;
	}
	// Assign socket_addr varibles
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addrLen = sizeof(serv_addr);
	// Attempt to bind to the socket.
	int bind_s = bind(sd, (struct sockaddr *) &serv_addr, addrLen);
	if(bind_s == -1){
		printf("Could not bind to socket. Please check open connections.\n");
		return -1;
	}
	s->server = serv_addr;
	s->serverLen = addrLen;
	return sd;

}


// Worker helper method :)
void * writeFileHelper2(void * s){

	struct socketlist * sock = (struct socketlist *)s;
	int sd = sock->client;
	struct sockaddr_in serv_addr = sock->server;
	socklen_t addrLen = sock->serverLen;
	char ipstr[INET6_ADDRSTRLEN];
	int status = listen(sd, MAX_CLIENTS);
	
	if(status < 0){
		printf("Could not listen\n");
		return 0;
	}
	// We listen until we accept a connection.
	int isaccepted = accept(sd, (struct sockaddr *) &serv_addr, &addrLen);
	// Information about client
	inet_ntop(AF_INET, &serv_addr.sin_addr, ipstr, sizeof(ipstr));


	if(isaccepted < 0){
		printf("PANIC CLIENT MULTI ERR\n");
		return 0;
	}
	errno = 0;
	size_t nbytes = sock->nbytes;
	char * readBuffer = sock->buffer;
	char * buffer = readBuffer;
	int readBytes = 0;
	while(readBytes < nbytes){
		int temp = 0;
		temp = recv(isaccepted, buffer, nbytes, 0);
		readBytes += temp;
		buffer += temp;
		if(temp == 0 && readBytes < nbytes){
			break;
		}
	}
	// send error
	int retError = send(isaccepted, &errno, sizeof(errno), 0);
	if(retError < 0){
		printf("Send error\n");
	}
	// remember to free later
	close(isaccepted);
	return 0;
}

// Helper method for writing over 2k bytes.
void writeFileHelper(int client,int filedes,size_t nbytes){
	
	int i = 0;
	int bindable = 0;
	int portsArr[MAX_SOCKETS];
	bzero(portsArr,sizeof(portsArr));
	pthread_t threadid[MAX_SOCKETS];
	struct socketlist * sock[MAX_SOCKETS];
	pthread_mutex_lock(&sockets);
	printf("PORTS: ");
	for(i = 0;i < MAX_SOCKETS;i++){
		if(bindable == MAX_STREAMS_PER_CLIENT){
			break;
		}
		if(socketL[i].clientsocket == 0){
			socketL[i].clientsocket = client;
			socketL[i].part = bindable;
			socketL[i].fd = filedes;
			sock[bindable] = &socketL[i];
			portsArr[bindable] = socketL[i].port;
			bindable++;
			printf("%d ",socketL[i].port);
		}
	}
	printf("\n");
	pthread_mutex_unlock(&sockets);
	if(bindable == 0){
		portsArr[0] = -1*ECONNRESET;
		send(client, &portsArr, sizeof(portsArr),0);
		printf("CANNOT BIND ANY MORE SOCKETS!\n");
		return;
	}
	errno = 0;

	int isFirst = 1;
	for(i = 0;i < bindable;i++){
		size_t segment = nbytes / bindable;
		if(isFirst){
			segment += nbytes % bindable;
			isFirst = 0;
		}
		char * buffer = calloc(1,segment);
		sock[i]->buffer = buffer;
		sock[i]->nbytes = segment;
	}
	// sending the ports to be used.
	if(errno != 0){
		pthread_mutex_lock(&sockets);
		for(i = 0;i<bindable;i++){
			sock[i]->clientsocket = 0;
		}
		pthread_mutex_unlock(&sockets);
		portsArr[0] = errno * -1;
		send(client, &portsArr, sizeof(portsArr),0);
		return;
	}
	// send ports to client
	int sendPorts = send(client, &portsArr, sizeof(portsArr),0);
	if(sendPorts < 0){
		printf("Sending error\n");
		return;
	}
	for(i = 0;i < bindable;i++){
		pthread_create(&threadid[i], NULL, writeFileHelper2, (void*)sock[i]);
	}
	for(i = 0;i < bindable;i++){
		pthread_join(threadid[i],0);
	}
	errno = 0;
	// do some negative errno stuff
	int reader = 0;
	int writer = 0;
	for(i = 0;i < bindable;i++){
		reader = write(filedes, sock[i]->buffer,sock[i]->nbytes);
		free(sock[i]->buffer);
		if(reader == -1){
			reader = -1*errno;
			writer = reader;
			break;
		}
		writer += reader;
	}


	if(debug) printf("\n\n<<<<<<<<<< writehelp1, writer: %d>>>>>>>>>>>>>\n\n\n",writer);
	// send bytes written
	int readBytes = send(client, &writer, sizeof(writer),0);
	if(debug) printf("\n\n<<<<<<<<<< writehelp1, writer: %d>>>>>>>>>>>>>\n\n\n",writer);
	if(readBytes < 0){
		printf("Sending error\n");
		return;
	}
	pthread_mutex_lock(&sockets);
	for(i = 0;i<bindable;i++){
		close(sock[i]->clientsocket);
		sock[i]->clientsocket = 0;
	}
	pthread_mutex_unlock(&sockets);
	return;
}

bool isitokToWrite(int fd, int *alreadyCaughtError){
	struct clientlist * clptr;
	struct filelist * flptr;
	int fmp1, fmp2;
	int nnode;
	int isFileModePermsOK;
	bool retBoolVal;

	if(debug){
		printf("isitokToReadWrite: fd=[%d] alreadyCaughtError=[%d]\n",
				fd, *alreadyCaughtError);
	}

	retBoolVal = true;
	fmp1=fmp2=-1;

	flptr = fL;
	clptr = flptr->list;

	if(clptr == NULL){
		errno = EBADF;
		retBoolVal = false;
	}
	else{
		while(flptr != NULL){
			clptr = flptr->list;
			while(clptr != NULL){
				if(clptr->fd == fd){
					fmp2 = clptr->fmp;
					nnode = flptr->nnode;

					if (debug) {
						printf("1. isitokToReadWrite fmp2=[%d] nnode=[%d]\n", fmp2, nnode);
					}

					if ( (fmp2 != U_W) && (fmp2 != U_RW) &&
							(fmp2 != E_W) && (fmp2 != E_RW) &&
							(fmp2 != T_W) && (fmp2 != T_RW) ) {
						*alreadyCaughtError=1;
						errno = EACCES;
						retBoolVal = false;
					} 
					break; 
				}
				clptr = clptr->next;
			} // end clptr while
			if((*alreadyCaughtError) == 1){
				break;
			}
			else {
				flptr = flptr->next;
			}
		} //end flptr while
		
		if( (retBoolVal) && (fmp2 != -1)){
			flptr = fL;
			while(flptr != NULL){
				clptr = flptr->list;
				if(flptr->nnode == nnode){
					while(clptr != NULL){
						if(clptr->fd == fd){
							fmp1 = clptr->fmp;
							isFileModePermsOK = isFileModePermsOKtoOpen(fmp1, fmp2);
							if(isFileModePermsOK == 0){
								*alreadyCaughtError = 1;
								errno = PERMS_ERROR;
								retBoolVal = false;
								break;
							}
						}
						clptr = clptr->next;
					} // end clptr while
				}
				if((*alreadyCaughtError) == 1){
					break;
				}
				else {
					flptr = flptr->next;
				}
			} //end while
		} //end retvalbool if
	} //end else

	return retBoolVal;
}


// Function to write file
int writeFile(int client){
	char * readBuffer;
	int fd;
	int error;
	size_t nbytes;
	int reader, alreadyCaughtError;
	// receive file descriptor
	int recfd = 0;
	struct filelist * flptr;
	struct clientlist * clptr;

	flptr = fL;

	 
	alreadyCaughtError = 0;	
	while(recfd < sizeof(int)){
		recfd += recv(client, &fd, sizeof(int), 0);
		if(recfd == 0){
			continue;
		}
	}
	
	if(debug) printf("In writeFile, fd: [%d]\n", fd);


	if(flptr != NULL){
		clptr = flptr->list;
		if(clptr->next != NULL){
		

			if(!(isitokToWrite(fd, &alreadyCaughtError))){
				if(debug) printf("Not Okay to Write :(\n");
				errno = EBADF;
				return -1;
			}
		}
	}
	// receive # of bytes to write
	int recnbyte = 0;
	while(recnbyte < sizeof(ssize_t)){
		recnbyte += recv(client, &nbytes, sizeof(ssize_t), 0);
		if(recnbyte == 0){
			continue;
		}
	}
	
	if(debug) printf("\n\nIn writeFile, nbytes = %zd\n\n", nbytes);	

	// if large bytes thread method
	if(nbytes > 4096){
		if(debug) printf("In nbytes > 4096, fd:[%d]\n", fd);
		if(fd % 5 == 0){
			fd /= -5;
		} else{
			reader = -1;
			printf("DO LATER\n");
		}
		
		writeFileHelper(client, fd,nbytes);
		printf("Client %d done writing\n",client);
		return 0;
	}

	//receive data to write
	readBuffer = calloc(1,nbytes);
	char * buffer = readBuffer;
	int readBytes = 0;
	while(readBytes < nbytes){
		int temp = 0;
		temp = recv(client, buffer, nbytes, 0);
		readBytes += temp;
		buffer += temp;
		if(temp == 0 && readBytes < nbytes){
			return 0;
		}
	}
	if(fd % 5 == 0){
		fd /= -5;
		printf("FD: %d\n",fd);
		reader = write(fd,readBuffer,nbytes);
		printf("Written Bytes: %d\n",reader);
	} else{
		reader = -1;
	}
	if(reader == -1){
		error = errno;
		if(error == 0){
			error = -1;
		}
	} else{
		error = 0;
	}
	// send error
	int retError = send(client, &error, sizeof(error), 0);
	if(reader == -1 || retError < 0){
		free(readBuffer);
		return -1;
	}
	// send bytes written
	int retBytes = send(client, &reader, sizeof(reader),0);
	if(retBytes < 0){
		printf("Sending error\n");
	}
	free(readBuffer);
	return 0;
}

// worker thread for ext B
void * readFileHelper2(void * s){
	struct socketlist * sock = (struct socketlist *)s;
	int sd = sock->client;
	struct sockaddr_in serv_addr = sock->server;
	socklen_t addrLen = sock->serverLen;
	char ipstr[INET6_ADDRSTRLEN];
	int status = listen(sd, MAX_CLIENTS);
	if(status < 0){
		printf("Could not listen\n");
		return 0;
	}
	// We listen until we accept a connection.
	int isaccepted = accept(sd, (struct sockaddr *) &serv_addr, &addrLen);
	// Information about client
	inet_ntop(AF_INET, &serv_addr.sin_addr, ipstr, sizeof(ipstr));

	if(isaccepted < 0){
		printf("[%d] %s\n",h_errno,strerror(h_errno));
		printf("MULTI ERROR CLIENT PANIC\n");
		return 0;
	}
	size_t nbytes = sock->nbytes;
	char * readBuffer = sock->buffer;
	int error = 0;
	int reader = sock->nread;
	// send error
	int retError = send(isaccepted, &error, sizeof(error), 0);
	if(reader == -1 || retError == -1){
		free(readBuffer);
		return 0;
	}
	// send bytes read
	int readBytes = send(isaccepted, &reader, sizeof(reader),0);
	if(readBytes == -1){
		free(readBuffer);
		printf("Error sending\n");
		return 0;
	}
	// send data
	int retnBytes = send(isaccepted, readBuffer, nbytes, 0);
	if(retnBytes == -1){
		free(readBuffer);
		printf("Error sending\n");
		return 0;
	}
	printf("Sent: %d\n",retnBytes);
	free(readBuffer);
	return 0;
}

// helper method for reading
void readFileHelper(int client,int filedes,size_t nbytes){
	int i = 0;
	int bindable = 0;
	int portsArr[MAX_SOCKETS];
	pthread_t threadid[MAX_SOCKETS];
	bzero(portsArr,sizeof(portsArr));
	struct socketlist * sock[MAX_SOCKETS];
	pthread_mutex_lock(&sockets);
	printf("PORTS: ");
	for(i = 0;i < MAX_SOCKETS;i++){
		if(bindable == MAX_STREAMS_PER_CLIENT){
			break;
		}
		if(socketL[i].clientsocket == 0){
			socketL[i].clientsocket = client;
			socketL[i].part = bindable;
			socketL[i].fd = filedes;
			sock[bindable] = &socketL[i];
			portsArr[bindable] = socketL[i].port;
			bindable++;
			printf("In netserver, readhelper, socketL[%d] = %d\n ",i,socketL[i].port);
			printf("In netserver, readhelper, socketL[%d].clientsock = %d\n", i, socketL[i].clientsocket);
			printf("In netserver, readhelper, socketL[%d].part = %d\n", i, socketL[i].part);
			printf("In netserver, readhelper, socketL[%d].fd = %d\n", i, socketL[i].fd);
		}
	}
	printf("\n");
	pthread_mutex_unlock(&sockets);
	if(bindable == 0){
		portsArr[0] = -1*ECONNRESET;
		send(client, &portsArr, sizeof(portsArr),0);
		printf("CANNOT BIND ANY MORE SOCKETS!\n");
		return;
	}
	errno = 0;
	int isFirst = 1;
	for(i = 0;i < bindable;i++){
		size_t segment = nbytes / bindable;
		if(isFirst){
			segment += nbytes % bindable;
			isFirst = 0;
		}
		char * buffer = calloc(1,segment);
		sock[i]->nread = read(filedes,buffer,segment);
		sock[i]->buffer = buffer;
		sock[i]->nbytes = segment;
	}
	// remember to send nbyte
	if(errno != 0){
		printf("[%d] %s\n",errno,strerror(errno));
		pthread_mutex_lock(&sockets);
		for(i = 0;i<bindable;i++){
			sock[i]->clientsocket = 0;
		}
		pthread_mutex_unlock(&sockets);
		portsArr[0] = errno * -1;
		send(client, &portsArr, sizeof(portsArr),0);
		return;
	}
	// send ports to client
	int sendPorts = send(client, &portsArr, sizeof(portsArr),0);
	if(sendPorts == -1){
		printf("Sending Error\n");
		return;
	}
	for(i = 0;i < bindable;i++){
		pthread_create(&threadid[i], NULL, readFileHelper2, (void*)sock[i]);
	}
	for(i = 0;i < bindable;i++){
		pthread_join(threadid[i],0);
	}
	pthread_mutex_lock(&sockets);
	for(i = 0;i<bindable;i++){
		sock[i]->clientsocket = 0;
	}
	pthread_mutex_unlock(&sockets);
	return;
}

// function for reading.
int readFile(int client){
	char * readBuffer;
	int fd;
	int error;
	size_t nbytes;
	int reader;
	// receive file descriptor
	int recfd = 0;
	while(recfd < sizeof(int)){
		recfd += recv(client, &fd, sizeof(int), 0);
		if(recfd == 0){
			continue;
		}
	}

	// receive # of bytes to read
	int recnbyte = 0;
	while(recnbyte < sizeof(size_t)){
		recnbyte += recv(client, &nbytes, sizeof(size_t), 0);
		if(recnbyte == 0){
			continue;
		}
	}
	
	
	// if large bytes thread method
	if(nbytes > 4096){
		if(debug) printf("\n\n\nIn readFile, inside 4096\n\n\n");
 
		if(fd % 5 == 0){
			fd /= -5;
		} else{
			reader = -1;
			printf("DO LATER\n");
		}
		readFileHelper(client, fd, nbytes);
		printf("Client %d done reading\n",client);
		return 0;
	}

	readBuffer = calloc(1,nbytes);
	if(fd % 5 == 0){
		fd /= -5;
		reader = read(fd,readBuffer,nbytes);
		printf("Read Bytes: %d\n",reader);
	} else{
		reader = -1;
	}
	if(reader == -1){
		error = errno;
		if(error == 0){
			error = -1;
		}
	} else{
		error = 0;
	}
	// send error
	int retError = send(client, &error, sizeof(error), 0);
	if(reader == -1){
		free(readBuffer);
		return -1;
	}
	// send bytes read
	int readBytes = send(client, &reader, sizeof(reader),0);
	// send data
	int retnBytes = send(client, readBuffer, nbytes, 0);
	if(retError == -1 || readBytes == -1 || retnBytes == -1){
		free(readBuffer);
		return -1;
	}
	printf("Sent: %d\n",retnBytes);
	free(readBuffer);
	return 0;
}

/*
   Adds a file to list clientlist. 

   Returns file descriptor on completion.

*/
/*
int addFileToList(struct filelist * ptr, int client, int mode, int flag){
	
	pthread_mutex_lock(&list);
	struct clientlist * curr = ptr->list;
	struct clientlist * prev = ptr->list;
	int fd = 0;
	if(curr == 0){
		ptr->list = calloc(1,sizeof(clientlist));
		curr = ptr->list;
		curr->client = client;
		gettimeofday(&curr->start, NULL);
		curr->mode = mode;
		curr->flag = flag;
		int pipe_stat = pipe(curr->fd);
		if(pipe_stat < 0){
			printf("ERROR PIPING???\n");
		}
		fd = (curr->fd)[0];
		pthread_mutex_unlock(&list);
		return fd;
	}
	for(;curr != 0;curr = curr->next){
		prev = curr;
	}
	prev->next = calloc(1,sizeof(clientlist));
	prev = prev->next;
	prev->client = client;
	gettimeofday(&prev->start, NULL);
	prev->mode = mode;
	prev->flag = flag;
	int pipe_stat2 = pipe(prev->fd);
	if(pipe_stat2 < 0){
		printf("ERROR PIPING???\n");
	}
	fd = (prev->fd)[0];
	pthread_mutex_unlock(&list);
	if(debug) printf("\n\nLeaving addfiletolist\n\n");
	return fd;	
}
*/
/*
   Given a node, find the nnode that contains node.

   If not found, add file to end of filelist.

   Returns filelist structure.

*/

struct filelist * lookup_file(char * filename, int node){
	
	struct filelist * curr = fL;
	struct filelist * prev = curr;
	pthread_mutex_lock(&list);
	for(;curr != NULL;curr = curr->next){
		if(curr->nnode == node){
			pthread_mutex_unlock(&list);
			if(debug) printf("Returning from lookup\n");
			return curr;
		}
		prev = curr;
	}
	
	pthread_mutex_unlock(&list);
	if(debug) printf("Returning from lookup\n");
	return prev;
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


void printcL(struct clientlist * clptr){
	printf("------------------------ printcL ---------------------------\n");
	while(clptr != NULL){
		printf("client:[%d] mode:[%d] flag:[%d] fd:[%d] fmp:[%d]\n", 
				clptr->client, clptr->mode, clptr->flag, clptr->fd, clptr->fmp);
		clptr = clptr->next;
	}
}

void printfL(){
	struct filelist * flptr = fL;
	printf("------------------------ printfL ---------------------------\n");
	while(flptr != NULL){
		printf("filename:[%s] nnode:[%d]\n", flptr->filename, flptr->nnode);
		printcL(flptr->list);
		flptr = flptr->next;
	}
}

int rw_conflict(char * filename, int mode, int flag, int client){

	
	struct stat filestat; 
	struct filelist * flptr = NULL;
	struct filelist * tempflptr = NULL;
	struct clientlist * clptr = NULL;
	struct clientlist * tempclptr = NULL;
	fileModePerms fmp;
	fileModePerms fmp1, fmp2;
	stat (filename, &filestat);  
	int nnode = filestat.st_ino;
	int fd = 0;
	int retValOfisFileModePermsOK = 0;
	pthread_mutex_lock(&lock);
	if(fL == NULL){
		fd = open(filename, flag);
		if(debug) printf("original f:[%d]\n", fd);
		if(fd < 0){
			printf("[%d] %s\n", errno, strerror(errno));
			pthread_mutex_unlock(&lock);
			return -1;
		}
		fd = fd * (-5);
		printf("Adding file to filelist\n");
		flptr = calloc(1,sizeof(filelist));
		flptr->filename = filename;
		flptr->nnode = nnode;
		flptr->next = NULL;
		fmp = mapFileModePerms(mode, flag);
		clptr = calloc(1, sizeof(clientlist));
		clptr->fd = fd;
		clptr->fmp = fmp;
		clptr->client = client;
		clptr->mode = mode;
		clptr->flag = flag;
		clptr->next = NULL;
		flptr->list = clptr;

		//assign flptr to global fL 
		fL = flptr;

		if(debug){
			printfL();
		}
		pthread_mutex_unlock(&lock);
		return fd;
	}
	else{
		// 1. at this point check if the file has been opened by some other client
		// 2. so we need to check the with which mode and file perms was the 
		// file opened by the client. loop thru the client list for the nnode
		retValOfisFileModePermsOK = 0;
		flptr = fL;
		clptr = NULL;
		fmp2 = mapFileModePerms(mode, flag) ;       // the new client's fmp
		while(flptr != NULL){
			if(strcmp(flptr->filename, filename) == 0){
				clptr = flptr->list;
				while(clptr != NULL){

					fmp1 = clptr->fmp;
					retValOfisFileModePermsOK = isFileModePermsOKtoOpen(fmp1, fmp2);
					if (retValOfisFileModePermsOK  == 0) {
						// then  it implies that the file was opened by fmp1 for 
						// exclsive write or exclsive read/write or transaction mode

						break;       // out for while 
					}
					clptr = clptr->next;
				} //clptr while
				break;
			} //strcmp filename

			flptr = flptr->next;
		} // end flptr while
		
		// At this point, if flptr is null => file has not been opened by anyone [yet].
		if(flptr == NULL && retValOfisFileModePermsOK  == 0){

			fd = open(filename, flag);
			if(debug) printf("2. original fd:[%d]\n", fd);
			if(fd < 0){
				printf("[%d] %s\n", errno, strerror(errno));
				pthread_mutex_unlock(&lock);
				return -1;
			}
			fd = fd * (-5);
			printf("Adding file to filelist\n");
			flptr = calloc(1,sizeof(filelist));
			flptr->filename = filename;
			flptr->nnode = nnode;
			fmp = mapFileModePerms(mode, flag);
			clptr = calloc(1, sizeof(clientlist));
			clptr->fd = fd;
			clptr->fmp = fmp;
			clptr->client = client;
			clptr->mode = mode;
			clptr->flag = flag;
			clptr->next = NULL;
			flptr->list = clptr;

			//assign flptr to global fL 
			tempflptr = fL;
			fL = flptr;
			flptr->next = tempflptr;

			if(debug){
				printfL();
			}
			pthread_mutex_unlock(&lock);
			return fd;
		}

		// 2. at this point if retValOfisFileModePermsOK  == 1 then
		//    open the file and add it to the client list 
		if ( retValOfisFileModePermsOK  == 1 ) {

			fd = open(filename,flag);    

			if (debug) {
				printf("openFile:2.  open fileDescriptor is [%d] and  errno is set to [%d]\n",fd, errno);
			}
			if (fd == -1  ) {
				if (debug) {
					printf("openFile: fd is -1 and errno is :%d\n",errno);
				}
				pthread_mutex_unlock(&lock);
				return(errno * -1);
			}
			// add the client file perms to  the list -- 
			fd = fd * (-5);
			if (debug) {
				printf("openFile:3.  open fileDescriptor is [%d] and  errno is set to [%d]\n",fd, errno);
			}
			fmp = mapFileModePerms(mode, flag);
			if(debug) printf("Before calloc.\n");
			clptr = calloc(1, sizeof(clientlist));
			if(debug) printf("After calloc.\n");
			clptr->fd = fd;
			clptr->fmp = fmp;
			clptr->client = client;
			clptr->mode = mode;
			clptr->flag = flag;
			if(debug) printf("clptr assignments suucess\n");
			tempclptr = flptr->list;
			flptr->list = clptr;
			clptr->next = tempclptr;
			if(debug) printf("Before printfL().\n");
			if(debug){
				printfL();
			}
			pthread_mutex_unlock(&lock);

			return fd;

		} else if (retValOfisFileModePermsOK  == 0) {
			errno = PERMS_ERROR;
			if (debug) {
				printf("openFile: fildes is -1 and errno is :%d\n",errno);
			}
			pthread_mutex_unlock(&lock);
			return(errno * -1);
		} // retValOfisFileModePermsOK  == 1  or 0

	}  // else


	// at this point it's a valid fileDescriptor and as we have to send 
	// over  a -ve fileDescriptor number
	pthread_mutex_unlock(&lock);
	errno = 0;
	return (fd);

}


//attemps to open a file.
int openFile(int client){
	char * path = calloc(1,256);
	int flags;
	int mode;
	int error;
	//int isConflict;
	int fd;
	// receive connection mode (exclusive, transaction, etc)
	int recmode = 0;
	while(recmode < sizeof(int)){
		recmode += recv(client, &mode, sizeof(int), 0);
		if(recmode == 0){
			continue;
		}
	}
	// receive flags(read only,write,wr)
	int recflags = 0;
	while(recflags < sizeof(int)){
		recflags += recv(client, &flags, sizeof(int), 0);
		if(recflags == 0){
			continue;
		}
	}
	//path name
	int recpath = 0;
	while(recpath < 256){
		int temp = 0;
		temp = recv(client, path, 256, 0);
		recpath += temp;
		if(temp == 0){
			continue;
		}
	}
	//check for modes
	if(is_file(path)){
		errno = 0;
		pthread_mutex_lock(&conflict);
		fd = rw_conflict(path,mode,flags, client);
		if(debug) printf("In openFile, fd:[%d]\n", fd);
		pthread_mutex_unlock(&conflict);
		if(errno != 0){
			error = errno;
			fd = -1;
			if(debug) printf("[%d] fd:[%d] %s\n",error, fd, strerror(errno));
		}

	}
	if(debug) printf("Flag: %d\nMode: %d\nFilename: %s\n",flags,mode,path);
	int retnBytes = send(client, &fd, sizeof(fd), 0);
	int retError = send(client, &error, sizeof(error), 0);
	if(retError == -1 || retnBytes == -1){
		printf("Error sending\n");
		return -1;
	}

	return 0;
}

/*
// Deletes a node from the linked list when a file is closed.
void deleteFileFromList(int pos){
	struct fdlist * curr = fdL;
	struct fdlist * prev = curr;
	pthread_mutex_lock(&lock);
	if(curr != NULL && curr->fd == pos){
		fdL = curr->next;
		free(curr);
		pthread_mutex_unlock(&lock);
		return;
	}

	while(curr != NULL && curr->fd != pos){
		prev = curr;
		curr = curr->next;
	}

	if(curr == NULL){
		pthread_mutex_unlock(&lock);
		return;
	}

	prev->next = curr->next;
	pthread_mutex_unlock(&lock);
	free(curr);
	return;
}
*/
// This is the close method. IT closes the file, or attemps to.
int closeFile(int client){
	int fd,origfd;
        int nnode ; 
	char * filename = NULL;
	int error = 0;
	int reader;

	int recfd = 0;
	int count = 0;
	struct filelist * flptr = fL;
	struct filelist * fl_curr;
	struct filelist * fl_currtmp;
	struct filelist * fl_prev;
	struct clientlist * cl_curr;
	struct clientlist * cl_currtmp;
	struct clientlist * cl_prev;
	struct clientlist * clptr;

	origfd = 0;
	fl_prev=NULL; 
        nnode = -1;
	if (fl_prev == NULL) {};
	// receive fd to close
	while(recfd < sizeof(int)){
		recfd += recv(client, &fd, sizeof(int), 0);
		if(recfd == 0){
			continue;
		}
	}
	if(debug) printf("In netclose, fd:[%d]\n", fd);

	while(flptr != NULL){
	        if(debug) printf("IN flptr while loop [%s]\n",flptr->filename);
		clptr = flptr->list;
		while(clptr != NULL){
	                if(debug) printf("IN clptr while loop [%d]\n",clptr->fd);
			if(fd == clptr->fd){
                                nnode = flptr->nnode;
				break;
			}
			clptr = clptr->next;
		}
		
		if(nnode != -1){
			break;
		}

		flptr = flptr->next;
	}
	
	if(debug) printf("After flptr while loop.  nnode = [%d]\n",nnode);
	if(nnode == -1){
		printf("No file...nnode is -1 at this point\n");
		error = EBADF;
		send(client, &error, sizeof(error), 0);
		return -1;
	}

	//close FD and delete node. prob need mutex
	if(fd % 5 == 0){
		origfd = fd/(-5);
		if (debug) printf("FD: origfd=[%d] fd=[%d]\n",origfd, fd);
		reader = close(origfd);
		if (debug) printf("AFTER FD: origfd=[%d] fd=[%d]\n",origfd, fd);
	} else{
		reader = -1;
	}
	if (debug) printf("0-1 reader=[%d]\n",reader);
	if(reader == -1){
		error = errno;
		if(error == 0){
			error = -1;
		}
	} else{
	       if (debug) printf("0-2 in else \n");

                // nnode was captured from above from the list
                /*
		struct stat filestat; 
		stat (filename, &filestat);  
		int nnode = filestat.st_ino;
                */

	       if (debug) printf("0-2 after stat  \n");
		flptr = lookup_file(filename, nnode);
	       if (debug) printf("0-3 after lookup  \n");
		fl_prev = fl_curr = fL;
		if(debug){
			 printf("0-4 : before starting the cleanup\n");
			 printfL();
			 printf("0-4 : end start \n");
		}

                
                // start of cleanup flptr->list 
		cl_curr = cl_prev = flptr->list;
		if(cl_curr == NULL){
			printf("NO QUEUE of client list..so not cleanup required of client list\n");
		}
		if(debug)printf("Before cl_curr for loop\n");

		while (cl_curr != NULL) {
                    if (cl_curr->fd == fd){ 

                        if (count == 0) {

                            if ( (cl_curr == cl_prev) && (cl_curr->next == NULL)) {
                                count = 1 ;
                                flptr->list = NULL;
                                cl_curr = NULL;
                                break; 
                            }  else {
                                count = 1;
                                flptr->list = cl_curr->next;
                                cl_currtmp = cl_curr; 
				cl_curr = cl_curr->next;
                                free(cl_currtmp);
                                break;
                            }
                            count = 1;
                        } else if (count ==1 ){
                            if (cl_curr->next == NULL) {
                                cl_prev->next = NULL; 
                                cl_currtmp = cl_curr; 
                                free(cl_currtmp);
                                break;
                            } else {
                                cl_prev->next = cl_curr->next;; 
                                cl_currtmp = cl_curr; 
                                free(cl_currtmp);
                                break;
                            }
                           
                        } // else of count is 1 and 0
                    }  // end if of fd == clptr->fd

                     
                    cl_prev = cl_curr;
                    cl_curr = cl_curr->next;
                    count =1 ;
                } // end while of cl_curr
		
		if(debug){
			 printf("0-5 : after cleanup of the cl_curr\n");
			 printfL();
			 printf("0-5 : end cleanup  \n");
		}
		if(debug)printf("After cl_curr for loop\n");
                //   ==================END OF CLEANUP flptr->list=============


                // fl_prev and fl_curr has been init earlier
		count = 0;
		if(debug) printf("nnode: [%d]\n", nnode);
		if(flptr->list == NULL){
			if(debug)printf("Before fl_curr for loop\n");

			while(fl_curr != NULL){
                             //pthread_mutex_lock(&list);
                             if (fl_curr->nnode == nnode){

                                 if (count == 0) { 

                                    if ( (fl_curr == fl_prev) && (fl_curr->next == NULL)) {
                                        count = 1;
                                        fL = NULL;                 // global variable fL
                                        fl_curr = NULL;
                                        break;
                                    } else {
                                       count = 1;
                                       fL = fl_curr->next;        // global variable fL
                                       fl_currtmp = fl_curr;
                                       fl_curr = fl_curr->next;
                                       free(fl_currtmp);
                                       break;
                                    }
                                    count = 1;
                                 }   else if (count == 1) {
                                       if (fl_curr->next == NULL) {
                                            fl_prev->next = NULL;
                                            fl_currtmp = fl_curr;
                                            free(fl_currtmp);
                                            break;
                                       } else {
                                            fl_prev->next = fl_curr->next;;
                                            fl_currtmp = fl_curr;
                                            free(fl_currtmp);
                                            break;
                                      }
                                 } // end else of count is 1 and 0

                             }  // end of if condn of fl_curr->nnode = nnodecheck of 
                            
                             //pthread_mutex_unlock(&list);
                             fl_prev = fl_curr;
                             fl_curr = fl_curr->next;
                             count = 1;
 
                        } // end while of fl_curr !=null

                        if(debug){
                              printf("0-6 : after cleanup of the fl_curr\n");
                              printfL();
                              printf("0-6 : end fl_curr cleanup  \n");
                        } 
                        if(debug)printf("After fl_curr for loop\n");
                        
		} 
		if(debug){
			printfL();
		}

	} // end else 
        
	// send error
	int retError = send(client, &error, sizeof(error), 0);
	if(retError < 0){
		printf("Sending error\n");
		return 0;
	}
	return error;
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
		case 1: if(debug) printf("in case 1: OPENING FILE\n");
			openFile(client);
			break;
		case 2: if(debug) printf("in case 2: READING FILE\n");
			readFile(client);
			break;
		case 3: if(debug) printf("In case 3: WRITING TO FILE\n");
			writeFile(client);
			break;
		case 4: if(debug) printf("In case 4: CLOSING FILE\n");
			closeFile(client);
			break;
	}

	close(client);
	free(c);

	if(debug){
		printf("In start: End of Request \n");
	}

	return 0;
}

int main(){
	
	signal(SIGPIPE, SIG_IGN);
	pthread_t threadid;
	int sd;
	int bind_socket;
	int status;
	int client;
	int * c;
	char ipstr[INET6_ADDRSTRLEN];
	struct sockaddr_in serv_addr;
	
	/* Create socket. */
	sd = socket(AF_INET, SOCK_STREAM, 0);

	/*Exit upon socket creation failure. */
	if(sd == -1){
		fprintf(stderr, "ERROR: Could not open socket, please check connection.\n");
		exit(1);
	}

	/* Make socket struct assignments. */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addrLen = sizeof(serv_addr);
	
	/* Bind sockaddr struct to socket. */
	bind_socket = bind(sd, (struct sockaddr *) &serv_addr, addrLen);

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
	//errno = 0;

	// lets setup the socket array
	int i = 0;
	int pp = PORT + 1;
	errno = 0;
	for(i = 0;i < MAX_SOCKETS;i++){
		socketL[i].port = pp;
		socketL[i].client = getclients(&socketL[i],pp);
		if(socketL[i].client == -1){
			printf("Could not bind a stream.\n");
			exit(1);
		}
		socketL[i].clientsocket = 0;
		pp++;
	}

	/* Start listening. */
	status = listen(sd, MAX_CLIENTS);

	/* Return 0 if listen() fails. */
	if(status == -1){
		fprintf(stderr, "Listener error.\n");
		return 0;
	}

	printf("Listening on port [%d]\n", PORT);
	
	/* Wait indefinitely until a connection is accepted. */
	while(1){
		client = accept(sd, (struct sockaddr *)&serv_addr, &addrLen);
		c = (int *)calloc(1, sizeof(int));
		*c = client;
		
		/* Client info. */
		inet_ntop(AF_INET, &serv_addr.sin_addr, ipstr, sizeof(ipstr));
		printf("Connection established with %s\n", ipstr);
		pthread_create(&threadid, NULL, start, (void *)c);

               if (debug ) {
                   printf( "Worker thread completed work...back to waiting on accept\n");
                }

	}
			
	return 0;
}

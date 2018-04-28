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
	// send bytes written
	int readBytes = send(client, &writer, sizeof(writer),0);
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

// Function to write file
int writeFile(int client){
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
	// receive # of bytes to write
	int recnbyte = 0;
	while(recnbyte < sizeof(ssize_t)){
		recnbyte += recv(client, &nbytes, sizeof(ssize_t), 0);
		if(recnbyte == 0){
			continue;
		}
	}

	// if large bytes thread method
	if(nbytes > 4096){
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

/*
   Given a node, find the nnode that contains node.

   If not found, add file to end of filelist.

   Returns filelist structure.

*/

struct filelist * lookup_file(char * filename, int node){
	
	struct filelist * curr = fL;
	struct filelist * prev = curr;
	pthread_mutex_lock(&list);
	for(;curr != 0;curr = curr->next){
		if(curr->nnode == node){
			pthread_mutex_unlock(&list);
			return curr;
		}
		prev = curr;
	}
	prev->next = calloc(1,sizeof(filelist));
	prev = prev->next;
	prev->filename = filename;
	prev->nnode = node;
	pthread_mutex_unlock(&list);
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

//extension A. Used to check transfer mode conflicts, and update queue/linked list
int rw_conflict(char * filename, int mode, int flag, int client, int rd){
	
	int mode0 = 0;
	struct stat filestat; 
	struct fdlist * curr = fdL;
	struct fdlist * possible = 0;
	struct filelist * ptr = 0;
	stat (filename, &filestat);  
	int nnode = filestat.st_ino;
	int mode1 = 0;
	int fd = 0;
	int write1 = 0;
	pthread_mutex_lock(&lock);
	if(fL == 0){
		printf("Adding file to list\n");
		fL = calloc(1,sizeof(filelist));
		fL->filename = filename;
		fL->nnode = nnode;
	}
	ptr = lookup_file(filename,nnode);

	for(;curr != 0;curr = curr->next){
		if(curr->next == 0){
			possible = curr;
		}
		if(curr->nnode == nnode && curr->mode == 0){
			write1 = 1;
			mode0 = 1;
			continue;
		}
		if(curr->nnode == nnode && curr->mode == 1 && curr->filemode == O_RDONLY){
			mode1 = 1;
			continue;
		}
		if(curr->nnode == nnode && curr->mode == 1 && curr->filemode != O_RDONLY){
			write1 = 1;
			mode1 = 1;
			continue;
		}

		if(curr->nnode == nnode && curr->mode == 2){
			if(rd == 0){
				fd = addFileToList(ptr,client,mode,flag);
				pthread_mutex_unlock(&lock);
				return fd;
			}
			pthread_mutex_unlock(&lock);
			return -1;
		}
	}
	if((mode1 || mode0) && mode == 2){
		if(rd == 0){
			fd = addFileToList(ptr,client,mode,flag);
			pthread_mutex_unlock(&lock);
			return fd;
		}
		pthread_mutex_unlock(&lock);
		return -1;
	}
	curr = possible;
	if(rd == 0){
		if((mode1 == 0) || (mode1 == 1 && write1 == 1 && flag == O_RDONLY) || (mode1 == 1 && write1 == 0) || curr == 0){
			errno = 0;
			int o_flag = open(filename,flag) * -5;
			if(errno != 0 || o_flag == -1){
				pthread_mutex_unlock(&lock);
				return -1;
			}
			if(curr == 0){
				fdL = calloc(1,sizeof(fdlist));
				possible = fdL;
			} else{
				possible->next = calloc(1,sizeof(fdlist));
				possible = possible->next;
			}
			possible->filename = filename;
			possible->nnode = nnode;
			possible->mode = mode;
			possible->filemode = flag;
			pthread_mutex_unlock(&lock);
			possible->fd = o_flag;
			return possible->fd;
		} else{
			fd = addFileToList(ptr,client,mode,flag);
			pthread_mutex_unlock(&lock);
			return fd;
		}
	} else{
		if((mode1 == 0) || (mode1 == 1 && write1 == 1 && flag == O_RDONLY) || (mode1 == 1 && write1 == 0) || curr == 0){
			pthread_mutex_unlock(&lock);
			return 0;
		} else{
			pthread_mutex_unlock(&lock);
			return -1;
		}
	}
	pthread_mutex_unlock(&lock);
	return -1;
	
}

// used as a debug to list open files.
void listfiles(){
	struct fdlist * curr = fdL;
	printf("-------FILES-------\n");
	for(;curr != NULL;curr = curr->next){
		printf("FILE: %s\nTRANSFER MODE: %d\nFD: %d\nFILE MODE: %d\n\n",curr->filename,curr->mode,curr->fd,curr->filemode);
	}
	printf("-------------------\n");
}

//attemps to open a file.
int openFile(int client){
	char * path = calloc(1,256);
	int flags;
	int mode;
	int error;
	int isConflict;
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
		isConflict = rw_conflict(path,mode,flags, client,0);
		pthread_mutex_unlock(&conflict);
		if(errno != 0){
			error = errno;
			printf("[%d] %s\n",errno,strerror(errno));
			fd = -1;
		}
		if(isConflict > 0 && errno == 0){
			while(1){
				int pipe = 0;
				printf("Conflict\n");
				printf("PIPE: %d\n",isConflict);
				read(isConflict,&pipe,sizeof(pipe));
				
				if(pipe == -1){
					printf("TIMEOUT\n");
					fd = -1;
					errno = EWOULDBLOCK;
					send(client, &fd, sizeof(fd), 0);
					send(client, &errno, sizeof(errno), 0);
					return errno;
				}
				printf("PP: %d\n",pipe);
				pthread_mutex_lock(&conflict2);
				isConflict = rw_conflict(path,mode,flags, client,0);
				printf("BAD %d\n",isConflict);
				pthread_mutex_unlock(&conflict2);
				if(isConflict <= 0){
					fd = isConflict;
					break;
				}
			}
			// permision denied fix
		} else{
			if(fd == -1){
				error = errno;
			} else{
				fd = isConflict;
			}
		}
	}else{
		fd = -1;
		error = errno;
	}
	printf("Flag: %d\nMode: %d\nFilename: %s\n",flags,mode,path);
	int retnBytes = send(client, &fd, sizeof(fd), 0);
	int retError = send(client, &error, sizeof(error), 0);
	if(retError == -1 || retnBytes == -1){
		printf("Error sending\n");
		return -1;
	}
	//listfiles();
	return fd;
}


/*
   For extension A.

   Checks for transfer mode conflicts, and also updates linked list.
*/

/*
int rdwr_conflict(char * filename, int mode, int flag, int clientfd, int rd){
	struct stat filestatus;
	struct fdlist * filedescrips = fdL;
	struct fdlist * candidate = 0;
	struct filelist * curr = 0;
	
	int tempmode = 0;
	int tempmode2 = 0;
	int wr = 0; 
	int nnode;
	int fd = 0; 
	int openfd; 
	
	stat(filename, &filestatus); 
	
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

	
	recmode = 0;
	while(recmode < sizeof(int)){
		recmode += recv(clientfd, &mode, sizeof(int), 0);
		if(recmode == 0){
			continue;
		}
	}
	
	
	recflags = 0;
	while(recflags < sizeof(int)){
		recflags += recv(clientfd, &flags, sizeof(int), 0);
		if(recflags == 0){
			continue;
		}
	}
	
	
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

	if(is_file(path)){

		if(debug) printf("\n\nIn is_file loop\n\n");
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
			if(debug) printf("\n\nIn conflict loop\n\n");
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
*/
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
		case 1: if(debug) printf("in case 1\n");
			openFile(client);
			break;
		case 2: if(debug) printf("in case 2\n");
			readFile(client);
			break;
		case 3: if(debug) printf("In case 3\n");
			writeFile(client);
			break;
	}

	close(client);
	free(c);
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
	}
			
	return 0;
}

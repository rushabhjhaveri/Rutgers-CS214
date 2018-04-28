#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>
#include<netdb.h>
#include<pthread.h>
#include<signal.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include "libnetfiles.h"

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

/*
   NO DEBUG: 0
   DEBUG: 1
*/
unsigned int DEBUG = 1;


//Return 0 if successful :)

/*
   In essence, netserverinit initializes the server. 

   Parameters: hostname and filemode [implemented for extension A].

   netserverinit checks if the filemode is valid, 
   else throws an invalid filemode error and returns failure.

   gethostbyname(const char * name) returns a hostent structure for the given 
   host name.

   If the hostname is valid and successfully retrieved, the global hostent
   structure gets populated. 
   Else, if null, a host not found error is thrown, and return failure.

   netserverinit returns 0 on success, -1 on failure / error, and sets 
   h_errno appropriately. 
*/
int netserverinit(char * hostname, int filemode){
	
	if(DEBUG){
		printf("0. In netserverinit, paramters: %s [%d]\n", hostname, filemode);
	}
	
	//check file mode
	if(filemode == 0 || filemode == 1 || filemode == 2){
		client_mode = filemode;
	}
	else{
		h_errno = INVALID_FILE_MODE;
		printf("ERROR: Invalid file mode.\n");
		return -1;
	}

	host_name = hostname;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	
	if(gethostbyname(hostname) == NULL){
		h_errno = HOST_NOT_FOUND;
		printf("ERROR: Host not found.\n");
		client_mode = -1;
		return -1;
	}

	if(DEBUG){
		printf("1. In netserverinit, host: %s , filemode: %d\n", host_name, client_mode);
	}

	host = gethostbyname(hostname);
	bcopy((char *)host->h_addr, (char *)&server.sin_addr.s_addr, host->h_length);

	return 0;
}

/*
   netopen(const char * filename, int flags)
   takes a filepath and flags, wherein free includes one of the
   following access modes: 
   	- O_RDONLY [read-only]
	- O_WRONLY [write-only]
	- O_RDWR [read-write]
   
   If the file was successfully opened, netopen() returns a new file descriptor,
   else, returns -1 and sets errno appropriately.

*/
int netopen(const char * pathname,int flags){
	
	/*
	   Check if flags are valid.
	   Set h_errno and return failure.
	*/
	if(flags < 0 || flags > 2){
		h_errno = INVALID_FILE_MODE;
		printf("Invalid flags.\n");
		return -1;
	}

	// Return failure if client_mode has been set to -1.
	if(client_mode == -1){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}
	char filename[256];
	bzero(filename,256);
	int fd;
	int error;
	int cmd = 1;
	strcpy(filename,pathname);
	// Create socket.
	sd = socket(AF_INET, SOCK_STREAM, 0);

	// Print errno error and exit if socket creation failed.
	if(sd < 0){
		printf("[%d] %s\n",errno,strerror(errno));
		exit(0);
	}
	serv_addrLen = sizeof(server);
	inet_ntop(AF_INET, &server.sin_addr, ip, sizeof(ip));

	// Connect to remote fileserver.
	int isConnected = connect(sd, (struct sockaddr *) &server, serv_addrLen);
	
	// Print errno error and exit if connection failed.
	if(isConnected < 0){
		errno = ETIMEDOUT;
		printf("[%d] %s\n",errno,strerror(errno));
		exit(1);
	}
	// Send command.
	int sendtype = send(sd, &cmd, sizeof(sendtype), 0);
	// Send client mode.
	int sendmode = send(sd, &client_mode, sizeof(client_mode), 0);
	// Send open flags
	int sendflags = send(sd, &flags, sizeof(flags), 0);
	// Send filepath.
	int sendpath = send(sd, filename, 256, 0);
	
	// Throw error, close socket, and exit, if any sends failed.
	if(sendtype == -1 || sendmode == -1 || sendflags == -1 || sendpath == -1){
		printf("Sending error\n");
		close(sd);
		return -1;
	}
	
	// Receive file descriptor returned by server.
	int retBytes = 0;
	while(retBytes < sizeof(fd)){
		retBytes += recv(sd, &fd, sizeof(fd), 0);
		if(retBytes == 0){
			printf("NO DATA\n");
			close(sd);
			return -1;
		}
	}

	// Receive error.
	int retError = 0;
	while(retError < sizeof(error)){
		retError += recv(sd, &error, sizeof(error), 0);
		if(retError == 0){
			printf("NO DATA\n");
			close(sd);
			return -1;
		}
	}

	printf("In, netopen, retBytes: %d, retError: %d\n", retBytes, retError);
	
	// If fd == -1, open failed; throw errno error.
	if(fd == -1){
		errno = error;
		printf("[%d] %s\n",fd,strerror(errno));
	}

	// Close the socket connection.
	close(sd);

	// Return the filedescriptor.
	return fd;
}

/*
   Helper function to make a socket, given a port number.

   It creates and binds the socket to the given port.

   Returns socket descriptor on success, throws errno error and exits on failure.
*/
int buildSocket(int portNum){
	
	struct sockaddr_in serv_addr;
	struct hostent * host;
	int sd = 0;
	int isconnected;
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portNum);
	host = gethostbyname(host_name);
	bcopy((char *) host->h_addr, (char *)&serv_addr.sin_addr.s_addr, host->h_length);

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1){
		printf("[%d] %s\n", errno, strerror(errno));
		exit(0);
	}
	
	serv_addrLen = sizeof(serv_addr);
	inet_ntop(AF_INET, &serv_addr.sin_addr, ip, sizeof(ip));
	isconnected = connect(sd, (struct sockaddr *) &serv_addr, serv_addrLen);
	
	if(isconnected == -1){
		errno = ETIMEDOUT;
		printf("[%d] %s\n", errno, strerror(errno));
		exit(1);
	}
	
	return sd;
}

void * netreadHelper(void * s){
	struct decipherSocket * msg = (struct decipherSocket *) s;
	int portNum = msg->port;
	int client = buildSocket(portNum);
	size_t nbyte = msg->nbytes;
	char * buffer = msg->buffer;
	int error;
	int recMsg;

	//receive error first
	int retError = 0;
	while(retError < sizeof(error)){
		retError += recv(client, &error, sizeof(error), 0);
		if(retError == 0){
			msg->error = ECONNRESET;
			close(sd);
			return 0;
		}
	}
	if(retError != sizeof(int)){
		printf("Could not receive error!\n");
		return 0;
	}
	if(error != 0){
		errno = error;
		printf("[%d] %s\n",error,strerror(errno));
		close(client);
		return 0;
	}
	// received byte number
	int recBytes = 0;
	while(recBytes < sizeof(recBytes)){
		recBytes += recv(client, &recMsg, sizeof(recMsg), 0);
		if(recBytes == 0){
			msg->error = ECONNRESET;
			close(client);
			return 0;
		}
	}
	msg->read = recMsg;
	int retnBytes = 0;
	errno = 0;
	while(retnBytes < nbyte){
		int temp;
		errno = 0;
		temp = recv(client, buffer, nbyte, 0);
		if(temp == 0 && retnBytes < nbyte){
			msg->error = ECONNRESET;
			close(client);
			return 0;
		} else if(temp == -1){
			msg->error = errno;
			close(client);
			return 0;
		}
		buffer += temp;
		retnBytes += temp;
	}
	printf("Received: %d/%zd\n",retnBytes,nbyte);
	if(error == -1){
		recBytes = -1;
	}
	close(client);
	return 0;
}

/*
   netread() reads the file referred to by fildes. 

   Returns the number of bytes actually read on success.
   Else, returns -1 and sets errno.
*/
ssize_t netread(int fildes, void *buf, size_t nbyte){
	signal(SIGPIPE, SIG_IGN);
	// Sets h_errno and returns -1 if clientmode is invalid.
	if(client_mode == -1){
		h_errno = HOST_NOT_FOUND;
		return -1;
	} else if((int)nbyte < 0){ // if number of bytes to be read is negative.
		// Set errno to invalid argument.
		errno = EINVAL;

		// Return -1 for failure.
		return -1;
	}
	/* 
	   Check if invalid filedescriptor or buffer.

	   Set errno to bad file descriptor and
	   return -1 for failure.
	*/
	if(fildes >=0 || fildes % 5 != 0 || buf == 0){
		errno = EBADF;
		return -1;
	}
	int error;
	int recMsg;
	int portsArr[MAX_SOCKETS];
	pthread_t threadid[MAX_SOCKETS];
	struct decipherSocket msg[MAX_SOCKETS];
	int cmd = 2;
	// Create socket.
	sd = socket(AF_INET, SOCK_STREAM, 0);

	// Print errno and exit if socket creation fails.
	if(sd < 0){
		printf("[%d] %s\n",errno,strerror(errno));
		exit(0);
	}
	serv_addrLen = sizeof(server);
	inet_ntop(AF_INET, &server.sin_addr, ip, sizeof ip);

	// Establish connection to remote fileserver.
	int status = connect(sd, (struct sockaddr *) &server, serv_addrLen);

	// Print errno and exit if connection fails.
	if(status < 0){
		errno = ETIMEDOUT;
		printf("[%d] %s\n",errno,strerror(errno));
		exit(1);
	}
	// Send command.
	int sendtype = send(sd, &cmd, sizeof(sendtype), 0);
	// Send fildes.
	int sendfd = send(sd, &fildes, sizeof(fildes), 0);
	//Send nbytes.
	int sendnbyte = send(sd, &nbyte, sizeof(nbyte), 0);

	// Throw error, close socket and return -1 if any send fails.
	if(sendtype == -1 || sendfd == -1 || sendnbyte == -1){
		printf("Sending error\n");
		close(sd);
		return -1;
	}
	int recPorts = 0;
	int temp = 0;
	if(nbyte > 4096){
		//receive ports first if size > 2048
		while(recPorts < sizeof(portsArr)){
			temp = recv(sd, &portsArr+temp, sizeof(portsArr), 0);
			recPorts += temp;
			if(recPorts == 0){
				errno = ECONNRESET;
				close(sd);
				return -1;
			}
		}
		int i = 0;
		int count = 0;
		printf("PORTS: ");
		for(i = 0;i < MAX_SOCKETS;i++){
			if(portsArr[i]> 0){
				printf("%d ",portsArr[i]);
				msg[i].port = portsArr[i];
				count++;
			} else if (portsArr[i] < 0){
				errno = portsArr[i]*-1;
				printf("[%d] %s\n",errno,strerror(errno));
				return -1;
			}
		}
		printf("\n");
	int isFirst = 1;
	for(i = 0;i < count;i++){
		size_t segment = nbyte / count;
		if(isFirst){
			segment += nbyte % count;
			isFirst = 0;
		}
		msg[i].error = 0;
		msg[i].buffer = calloc(1,segment);
		msg[i].nbytes = segment;
	}
		// make sock
		for(i = 0;i < count;i++){
				pthread_create(&threadid[i], NULL, netreadHelper, (void*)&msg[i]);
		}
		for(i = 0;i<count;i++){
			if(threadid[i] != 0){
				pthread_join(threadid[i],0);
			}
		}
		int reader = 0;
		char * tempbuffer = buf;
		for(i = 0;i<count;i++){
			if(threadid[i] != 0){
				if(msg[i].error != 0){
					errno = msg[i].error;
					return -1;
				}
				reader += msg[i].read;
				memcpy(tempbuffer,msg[i].buffer,msg[i].nbytes);
				free(msg[i].buffer);
				tempbuffer += msg[i].nbytes;
			}
		}
		return reader;
	} //if 4096
	
	// Receive error.
	int retError = 0;
	while(retError < sizeof(error)){
		retError += recv(sd, &error, sizeof(error), 0);
		if(retError == 0){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
	}
	if(retError != sizeof(int)){
		printf("Could not receive error!\n");
		return -1;
	}
	if(error != 0){
		errno = error;
		printf("[%d] %s\n",error,strerror(errno));
		close(sd);
		return -1;
	}
	// Receive bytes read.
	int recBytes = 0;
	while(recBytes < sizeof(recBytes)){
		recBytes += recv(sd, &recMsg, sizeof(recMsg), 0);
		if(recBytes == 0){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
	}

	int retnBytes = 0;
	errno = 0;

	// Receive populated buffer.
	while(retnBytes < nbyte){
		int temp;
		errno = 0;
		temp = recv(sd, buf, nbyte, 0);
		if(temp == 0 && retnBytes < nbyte){
			errno = ECONNRESET;
			close(sd);
			return -1;
		} else if(temp == -1){
			close(sd);
			return -1;
		}
		buf += temp;
		retnBytes += temp;
		printf("Received: %d/%zd\n",retnBytes,nbyte);
	}
	if(error == -1){
		recMsg = -1;
	}
	close(sd);

	//Return number of bytes read.
	return recMsg;
}

// netwrite helper method.
void * netwriteHelper(void * s){
	struct decipherSocket * msg = (struct decipherSocket *) s;
	int portNum = msg->port;
	int client = buildSocket(portNum);
	size_t nbyte = msg->nbytes;
	char * buffer = msg->buffer;
	int error;
	// send data
	errno = 0;
	int data = send(client, buffer, nbyte, 0);
	if(data == -1){
		msg->error = ECONNRESET;
		close(client);
		return 0;
	}
	if(errno == SIGPIPE || errno != 0){
		if(errno == SIGPIPE){
			msg->error = ECONNRESET;
			return 0;
		} else{
			msg->error = errno;
			return 0;
		}
	}
	//receive error first
	int retError = 0;
	while(retError < sizeof(error)){
		retError += recv(client, &error, sizeof(error), 0);
		if(retError == 0){
			msg->error = ECONNRESET;
			close(client);
			return 0;
		}
	}
	if(retError != sizeof(int)){
		printf("Could not receive error!\n");
		return 0;
	}
	if(error != 0){
		errno = error;
		printf("[%d] %s\n",error,strerror(errno));
		close(client);
		return 0;
	}
	close(client);
	return 0;
}

//netwrite attemps to write bytes to remote host
ssize_t netwrite(int fildes, const void *buf, size_t nbyte){
	signal(SIGPIPE, SIG_IGN);

	// remember to set errno
	if(client_mode == -1){
		h_errno = HOST_NOT_FOUND;
		return -1;
	} else if((int)nbyte < 0){
		errno = EINVAL;
		return -1;
	}
	// remember to set errno
	if(fildes >=0 || fildes % 5 != 0 || buf == 0){
		errno = EBADF;
		return -1;
	}
	int error;
	int recMsg;
	int recPorts = 0;
	int temp = 0;
	int portsArr[MAX_SOCKETS];
	struct decipherSocket msg[MAX_SOCKETS];
	pthread_t threadid[MAX_SOCKETS];
	int cmd = 3;
	// init server addr and client addr
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd < 0){
		printf("[%d] %s\n",errno,strerror(errno));
		exit(0);
	}
	serv_addrLen = sizeof(server);
	inet_ntop(AF_INET, &server.sin_addr, ip, sizeof ip);
	int isConnected = connect(sd, (struct sockaddr *) &server, serv_addrLen);
	if(isConnected < 0){
		errno = ETIMEDOUT;
		printf("[%d] %s\n",errno,strerror(errno));
		exit(1);
	}
	// send type
	int sendtype = send(sd, &cmd, sizeof(sendtype), 0);
	// send fdes
	int sendfd = send(sd, &fildes, sizeof(fildes), 0);
	//send nbytes
	int sendnbyte = send(sd, &nbyte, sizeof(nbyte), 0);
	if(sendtype == -1 || sendfd == -1 || sendnbyte == -1){
		printf("Sending error\n");
		close(sd);
		return -1;
	}
	if(nbyte > 4096){
		//receive ports
		while(recPorts < sizeof(portsArr)){
			temp = recv(sd, &portsArr+recPorts, sizeof(portsArr), 0);
			recPorts += temp;
			if(recPorts == 0){
				errno = ECONNRESET;
				close(sd);
				return -1;
			}
		}
		int i = 0;
		int count = 0;
		printf("PORTS: ");
		for(i = 0;i < MAX_SOCKETS;i++){
			if(portsArr[i]> 0){
				printf("%d ",portsArr[i]);
				msg[i].port = portsArr[i];
				count++;
			}else if (portsArr[i] < 0){
				errno = portsArr[i]*-1;
				printf("[%d] %s\n",errno,strerror(errno));
				return -1;
			}
		}
		printf("\n");
	char * buffer = (char*)buf;
	int isFirst = 1;
	for(i = 0;i < count;i++){
		size_t segment = nbyte / count;
		if(isFirst){
			segment += nbyte % count;
			isFirst = 0;
		}
		msg[i].error = 0;
		msg[i].buffer = buffer;
		msg[i].nbytes = segment;
		buffer += segment;
	}
		// make sock
		for(i = 0;i < count;i++){
				pthread_create(&threadid[i], NULL, netwriteHelper, (void*)&msg[i]);
		}
		for(i = 0;i<count;i++){
			if(threadid[i] != 0){
				pthread_join(threadid[i],0);
			}
		}
		for(i = 0;i < count;i++){
			if(msg[i].error != 0){
				errno = msg[i].error;
				return -1;
			}
		}
		//receive bytes written
		int recBytes = 0;
		while(recBytes < sizeof(recBytes)){
			recBytes += recv(sd, &recMsg, sizeof(recMsg), 0);
			if(recBytes == 0){
				errno = ECONNRESET;
				close(sd);
				return -1;
			}
		}
		if(recMsg < 0){
			errno = -1*recMsg;
			printf("[%d] %s\n",errno,strerror(errno));
			return -1;
		}
		return recMsg;
	}


	// send data
	errno = 0;
	int sent_data = send(sd, buf, nbyte, 0);
	if(errno == SIGPIPE){
		errno = ECONNRESET;
	} else if(sent_data == -1){
		return -1;
	}
	//receive error first
	int retError = 0;
	while(retError < sizeof(error)){
		retError += recv(sd, &error, sizeof(error), 0);
		if(retError == 0){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
	}
	if(retError != sizeof(int)){
		printf("Could not receive error!\n");
		return -1;
	}
	if(error != 0){
		errno = error;
		printf("[%d] %s\n",error,strerror(errno));
		close(sd);
		return -1;
	}
	// received byte number
	int recBytes = 0;
	while(recBytes < sizeof(recBytes)){
		recBytes += recv(sd, &recMsg, sizeof(recMsg), 0);
		if(recBytes == 0){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
	}
	if(error == -1){
		recMsg = -1;
	}
	close(sd);
	return recMsg;
}

/*
int netclose(int fildes){

}
*/
int main (){
	char * hostname = "localhost";
	netserverinit(hostname,1);
	
	char * filename = "file.txt";
	int fdd = netopen(filename,2);
	FILE * file = fopen(filename, "r");
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	printf("SIZE: %d\n",size);
	
	
	char * filename2 = "file2.txt";
	int fdd2 = netopen(filename2,2);
	FILE * file2 = fopen(filename2, "r");
	fseek(file2, 0L, SEEK_END);
	int size2 = ftell(file2);
	printf("SIZE2: %d\n",size2);
	
	
	char * txt = calloc(1,max(size,size2));
	printf("Read: %zd\n",netread(fdd,txt,size));
	printf("Write: %zd\n",netwrite(fdd2,txt,size));
	printf("[%d] %s\n",errno,strerror(errno));
	//int fcc =  netclose(fdd);
	//printf("Netclose %d\n[%d] %s\n",fcc,errno,strerror(errno));
	free(txt);
	return 0;
}

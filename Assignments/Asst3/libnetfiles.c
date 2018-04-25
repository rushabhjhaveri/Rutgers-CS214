#include<stdio.h>
#include<stdlib.h>
#include<string.h>
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
/*
   NO DEBUG: 0
   DEBUG: 1
*/
unsigned int DEBUG = 1;


//Return 0 if successful :)
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
	
	//check if hostname is valid
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

int netopen(const char * pathname, int flags){
	
	int fd = -1;
	char filename[256];
	int error;
	int type = 1;
	int isconnected;
	int type2;
	int mode;
	int o_flags;
	int o_path;
	int retnbytes;
	int retnerror;

	if(DEBUG){
		printf("0. In netopen, paramters: %s [%d]\n", pathname, flags);
	}
	if(flags < 0 || flags > 2){
		h_errno = INVALID_FILE_MODE;
		printf("ERROR: Invalid open flag.\n");
		return -1;
	}

	if(client_mode == -1){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}
	
	bzero(filename, 256);
	strcpy(filename, pathname);
	if(DEBUG){
		printf("1. In netopen, filename: %s\n", filename) ;
	}

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1){
		printf("[%d] %s\n", errno, strerror(errno));
		exit(0);
	}

	serv_addrLen = sizeof(server);
	inet_ntop(AF_INET, &server.sin_addr, ip, sizeof(ip));
	isconnected = connect(sd, (struct sockaddr *)&server, serv_addrLen);
	if(DEBUG){
		printf("2. In netopen, socket connnecting...........\n");
	}
	if(isconnected == -1){
		errno = ETIMEDOUT;
		printf("[%d] %s\n", errno, strerror(errno));
		exit(1);
	}

	if(DEBUG){
		printf("3. In netopen, socket connected and isconnected: %d\n", isconnected);
	}
	type2 = send(sd, &type, sizeof(type2), 0);
	mode = send(sd, &mode, sizeof(mode), 0);
	o_flags = send(sd, &o_flags, sizeof(flags), 0);
	o_path = send(sd, filename, 256, 0);
	if(type2==-1 || mode == -1 || o_flags == -1 ||o_path == -1){
		printf("ERROR: Message cannot be sent\n");
		close(sd);
		return -1;
	}

	if(DEBUG){
		printf("4. In netopen, type2: %d, mode: %d, o_flags: %d, o_path: %d\n", type2, mode, o_flags, o_path);
	}
	retnbytes = 0;
	while(retnbytes < sizeof(fd)){
		retnbytes += recv(sd, &fd, sizeof(fd), 0);
		if(retnbytes == 0){
			printf("ERROR: No data received\n");
			close(sd);
			return -1;
		}
	}

	retnerror = 0;
	while(retnerror < sizeof(error)){
		retnerror += recv(sd, &error, sizeof(error), 0);
		if(retnerror == 0){
			printf("ERROR: No data received\n");
			close(sd);
			return -1;
		}
	}

	if(DEBUG){
		printf("5. In netopen, \nretnbytes: %d, fd: %d \nretnerror: %d, error: %d\n", retnbytes, fd, retnerror, error);
	}

	if(fd == -1){
		errno = error;
		printf("[%d] %s\n", fd, strerror(errno));
	}

	if(DEBUG){
		printf("6. In netopen, returning fd value: %d\n", fd);
	}
	close(sd);
	return fd;
}

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

void * netreadHelper (void * msg){
	
	struct decipherSocket * dS = (struct decipherSocket *) msg;
	int portNum;
	int client;
	size_t nbytes;
	char * buffer;
	int error;
	int retError = 0;
	int recMsg = 0;
	int recBytes = 0;
	int retBytes = 0;

	portNum = dS->port;
	client = buildSocket(portNum);
	nbytes = dS->nbytes;
	buffer = dS->buffer;

	while(retError < sizeof(error)){
		retError += recv(client, &error, sizeof(error), 0);
		if(retError == 0){
			dS->error = ECONNRESET;
			close(sd);
			return 0;
		}
	}

	if(retError != sizeof(int)){
		if(DEBUG) printf("ERROR: Cannot send\n");
		return 0;
	}

	if(error != 0) {
		errno = error;
		printf("[%d] %s\n", error, strerror(errno));
		close(client);
		return 0;
	}

	while(recBytes < sizeof(recBytes)){
		recBytes += recv(client, &recMsg, sizeof(recMsg), 0);
		if(recBytes == 0){
			dS->error = ECONNRESET;
			close(client);
			return 0;
		}
	}

	dS->read = recMsg;
	errno = 0;

	while(retBytes < nbytes){
		int temp;
		errno = 0;
		temp = recv(client, buffer, nbytes, 0);
		if(temp == 0 && retBytes < nbytes) {
			dS->error = ECONNRESET;
			close(client);
			return 0;
		}
		else if (temp == -1) {
			dS->error = errno;
			close(client);
			return 0;
		}
		buffer += temp;
		retBytes += temp;
	}

	if(DEBUG) printf("In netreadHelper, buff receieved! retBytes: %d, nbytes: %zd\n", retBytes, nbytes);

	if(error == -1) {
		recBytes = -1;
	}
	close(client);
	return 0;
}

/*
   Read n bytes from remote fileserver.

   Returns number of bytes actually read upon success.

   Returns -1 and sets errno in case of failure.
*/
ssize_t netread(int fildes, void * buf, size_t nbyte){
	
	int error;
	int portArr[MAX_SOCKETS];
	int isconnected;
	int reader;
	int sendcmd;
	int sendfd;
	int sendnbytes;
	int recBytes;
	int recPorts;
	int recError;
	int retBytes;
	int retNBytes;
	int temp;
	int i;
	int div;
	int isFirst;
	int cmd = 2;
	size_t segment;
	char * tempbuffer;
	pthread_t threadid[MAX_SOCKETS];
	struct decipherSocket dS[MAX_SOCKETS];
	
	/*
	   Ignores the signal SIGPIPE.
	   Passing SIG_IGN as handler ignores a given signal, with exceptions:
	   	- SIGKILL
		- SIGSTOP
	   which cannot be caught or ignored.
	*/
	signal(SIGPIPE, SIG_IGN);
	
	/* Set errno. */
	if(client_mode == -1){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}
	else if((int) nbyte < 0){
		errno = EINVAL;
		return -1;
	}

	if(fildes >= 0 || fildes % 5 != 0 || buf == 0){
		errno = EBADF;
		return -1;
	}

	/* Initialize server and client sockets. */
	sd = socket(AF_INET, SOCK_STREAM, 0);

	/* Set errno, print error, and exit if socket creation failed. */
	if(sd == -1){
		printf("[%d] %s\n", errno, strerror(errno));
		exit(0);
	}

	serv_addrLen = sizeof(server);

	inet_ntop(AF_INET, &server.sin_addr, ip, sizeof(ip));

	isconnected = connect(sd, (struct sockaddr *) &server, serv_addrLen);

	/* Set errno, print error, and exit if connect failed. */
	if(isconnected == -1){
		errno = ETIMEDOUT;
		printf("[%d] %s\n", errno, strerror(errno));
		exit(1);
	}

	/* Send command type. */
	sendcmd = send(sd, &cmd, sizeof(sendcmd), 0);

	/*Send fildes. */
	sendfd = send(sd, &fildes, sizeof(fildes), 0);

	/* Send nbytes. */
	sendnbytes = send(sd, &nbyte, sizeof(nbyte), 0);

	/* Check if any of the sends failed. Throw error, close socket, return -1. */
	if(sendcmd == -1 || sendfd == -1 || sendnbytes == -1){
		if(DEBUG){
			printf("In netread, sending error. \n");
		}
		close(sd);
		return -1;
	}

	/* If nbyte > 4096, receive ports first. */
	recPorts = 0;
	temp = 0;
	if(nbyte > 4096){
		while(recPorts < sizeof(portArr)){
			temp = recv(sd, &portArr+temp, sizeof(portArr), 0);
			recPorts += temp;
			if(recPorts == 0){
				errno = ECONNRESET;
				close(sd);
				return -1;
			}
		}
		i= 0;
		div = 0;
		if(DEBUG){
			printf("Ports: ");
		}

		for(i = 0; i < MAX_SOCKETS; i++){
			if(portArr[i] > 0){
				if(DEBUG){
					printf("[%d] ", portArr[i]);
				}
				dS[i].port = portArr[i];
				div++;
			}
			else if(portArr[i] < 0){
				errno = portArr[i] * (-1);
				printf("[%d] %s\n", errno, strerror(errno));
				return -1;
			}
		}
		if(DEBUG){
			printf("\n");
		}

		isFirst = 1;
		for(i = 0; i < div; i++){
			segment = nbyte/div;
			if(isFirst){
				segment += nbyte % div;
				isFirst = 0;
			}

			dS[i].error = 0;
			dS[i].buffer = calloc(1, segment);
			dS[i].nbytes = segment;
		}

		for(i = 0; i < div; i++){
			pthread_create(&threadid[i], NULL, netreadHelper, (void *)&dS[i]);
		}

		for(i = 0; i < div; i++){
			if(threadid[i] != 0){
				pthread_join(threadid[i], 0);
			}
		}

		reader = 0;
		tempbuffer = buf;
		for(i = 0; i < div; i++){
			if(threadid[i] != 0){
				if(dS[i].error != 0){
					errno = dS[i].error;
					return -1;
				}

				reader += dS[i].read;
				memcpy(tempbuffer, dS[i].buffer, dS[i].nbytes);
				free(dS[i].buffer);
				tempbuffer += dS[i].nbytes;
			}
		}

		return reader;
	} //if 4096

	/* Receive error. */
	recError = 0;
	while(recError < sizeof(error)){
		recError += recv(sd, &error, sizeof(error), 0);
		if(recError == 0){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
	}

	if(recError != sizeof(int)){
		if(DEBUG){
			printf("In netread, error could not be received.\n");
		}
		return -1;
	}

	if(error != 0){
		errno = error;
		printf("[%d] %s\n", error, strerror(errno));
		close(sd);
		return -1;
	}
	
	/* Receive number of bytes. */
	retBytes = 0;
	while(retBytes < sizeof(retBytes)){
		retBytes += recv(sd, &recBytes, sizeof(recBytes), 0);
		if(retBytes == 0){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
	}

	retNBytes = 0;
	errno = 0;
	while(retNBytes < nbyte){
		errno = 0;
		temp = recv(sd, buf, nbyte, 0);
		if(temp == 0 && retNBytes < nbyte){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
		else if(temp == -1){
			close(sd);
			return -1;
		}

		buf += temp;
		retNBytes += temp;
		if(DEBUG){
			printf("In netread, received: %d / %zd\n", retNBytes, nbyte);
		}
	}

	if(error == -1){
		recBytes = -1;
	}

	close(sd);
	return recBytes;
}

void * netwriteHelper(void * msg){
	struct decipherSocket * dS = (struct decipherSocket *) msg;
	int error;
	int senddata;
	int retError;
	int port = dS->port;
	int clientfd = buildSocket(port);
	size_t nbyte = dS->nbytes;
	char * buffer = dS->buffer;
	
	errno = 0;

	senddata = send(clientfd, buffer, nbyte, 0);

	if(senddata == -1){
		dS->error = ECONNRESET;
		close(clientfd);
		return 0;
	}
	
	if(errno == SIGPIPE || errno != 0){
		if(errno == SIGPIPE){
			dS->error = ECONNRESET;
			return 0;
		}
		else{
			dS->error = errno;
			return 0;
		}
	}

	retError = 0;
	while(retError < sizeof(error)){
		retError += recv(clientfd, &error, sizeof(error), 0);
		if(retError == 0){
			dS->error = ECONNRESET;
			close(clientfd);
			return 0;
		}
	}

	if(retError != sizeof(int)){
		if(DEBUG){
			printf("Error: Could not receive error.\n");
		}
		return 0;
	}

	if(error != 0){
		errno = error;
		printf("[%d] %s\n", error, strerror(errno));
		close(clientfd);
		return 0;
	}

	close(clientfd);

	return 0;
}
ssize_t netwrite(int fildes, const void * buf, size_t nbyte){
	int error = 0;
	int recMsg = 0;
	int recBytes = 0;
	int recPorts = 0;
	int temp = 0;
	int cmd = 3;
	int sendtype;
	int sendfd;
	int sendnbytes;
	int senddata;
	int retError;
	int i;
	int div;
	int isFirst;
	int portArr[MAX_SOCKETS];
	size_t segment;
	char * buffer;
	struct decipherSocket dS[MAX_SOCKETS];
	pthread_t threadid[MAX_SOCKETS];

	
	signal(SIGPIPE, SIG_IGN);

	if(client_mode == -1){
		h_errno = HOST_NOT_FOUND;
		return -1;
	}
	else if((int) nbyte < 0){
		errno = EINVAL;
		return -1;
	}

	if(fildes >= 0 || fildes % 5 != 0 || buf == 0){
		errno = EBADF;
		return -1;
	}

	sd = socket(AF_INET, SOCK_STREAM, 0);

	if(sd == -1){
		printf("[%d] %s\n", errno, strerror(errno));
		exit(0);
	}

	sendtype = send(sd, &cmd, sizeof(sendtype), 0);
	sendfd = send(sd, &fildes, sizeof(fildes), 0);
	sendnbytes = send(sd, &nbyte, sizeof(nbyte), 0);

	if(sendtype == -1 || sendfd == -1 || sendnbytes == -1){
		if(DEBUG){
			printf("Error: Could not send.\n");
		}
		close(sd);
		return -1;
	}

	if(nbyte > 4096){
		while(recPorts < sizeof(portArr)){
			temp = recv(sd, &portArr+temp, sizeof(portArr), 0);
			recPorts += temp;
			if(recPorts == 0){
				errno = ECONNRESET;
				close(sd);
				return -1;
			}
		}
		
		i = 0;
		div = 0;
		if(DEBUG){
			printf("Ports: \n");
		}

		for(i = 0; i < MAX_SOCKETS; i++){
			if(portArr[i] > 0){
				if(DEBUG){
					printf("%d ", portArr[i]);
				}
				dS[i].port = portArr[i];
				div++;	
			}
			else if(portArr[i] < 0){
				errno = portArr[i] * (-1);
				printf("[%d] %s\n", errno, strerror(errno));
				return -1;
			}
		}
		if(DEBUG){
			printf("\n");
		}
		buffer = (void *) buf;
		isFirst = 1;
		for(i = 0; i < div; i++){
			segment = nbyte / div;
			if(isFirst){
				segment += nbyte % div;
				isFirst = 0;
			}
			dS[i].error = 0;
			dS[i].buffer = buffer;
			dS[i].nbytes = segment;
			buffer += segment;
		}

		for(i = 0; i < div; i++){
			pthread_create(&threadid[i], NULL, netwriteHelper, (void *)&dS[i]);
		}

		for(i = 0; i < div; i ++){
			if(threadid[i] != 0){
				pthread_join(threadid[i], 0);
			}
		}

		for(i = 0; i < div; i++){
			if(dS[i].error != 0){
				errno = dS[i].error;
				return -1;
			}
		}

		while(recBytes < sizeof(recBytes)){
			recBytes += recv(sd, &recMsg, sizeof(recMsg), 0);
			if(recBytes == 0){
				errno = ECONNRESET;
				close(sd);
				return -1;
			}
		}

		if(recMsg < 0){
			errno = (-1)*recMsg;
			printf("[%d] %s\n", errno, strerror(errno));
			return -1;
		}

		return recMsg;
		
	} /* End 4096. */

	errno = 0;
	senddata = send(sd, buf, nbyte, 0);
	if(errno == SIGPIPE){
		errno = ECONNRESET;
	}
	else if(senddata == -1){
		return -1;
	}

	retError = 0;
	while(retError < sizeof(error)){
		retError += recv(sd, &error, sizeof(error), 0);
		if(retError == 0){
			errno = ECONNRESET;
			close(sd);
			return -1;
		}
	}

	if(retError != sizeof(int)){
		if(DEBUG){
			printf("Error: Could not receive error.\n");
		}
		return -1;
	}

	if(error != 0){
		errno = error;
		printf("[%d] %s\n", error, strerror(errno));
		close(sd);
		return -1;
	}

	recBytes = 0;
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

int main (){
	/*
	int net; 
	net = netserverinit("factory.cs.rutgers.edu", 0);
	if(DEBUG){
		printf("0. In main, net: %d\n", net);
	}
	
	int o_fd;
	o_fd = netopen("file.txt", 0);
	if(DEBUG){
		printf("1. In main, netopen fd: %d\n", o_fd);
	}
	*/
	char * hostname = "localhost";
	netserverinit(hostname, 1);
	char * filename = "netfileserver.c";
	int fd = netopen(filename, 2);
	printf("in libnet main, fd: [%d]\n", fd);
	FILE * file = fopen(filename, "r");
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	printf("SIZE: %d\n", size);
	char * txt = calloc(1, size);
	printf("READ: %zd\n", netread(fd, txt, size));
	//printf("BUFFER: %s\n", txt);
	free(txt);
	return 0;
}

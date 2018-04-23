#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<netdb.h>
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
	char * filename = "file.txt";
	int fd = netopen(filename, 2);
	printf("in libnet main, fd: [%d]\n", fd);
	return 0;
}

#ifndef _LIBNETFILES_H_
#define _LIBNETFILES_H_

#define PORT 9697
#define MAX_BUFFER_SIZE 1024
#define MAX_SOCKETS 5
#define INVALID_FILE_MODE 500
#define PERMS_ERROR 100000
#define UNRESTRICTED 0
#define EXCLUSIVE 1
#define TRANSACTION 2

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
#include<fcntl.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<arpa/inet.h>
//#include "libnetfiles.h"
extern int h_errno;
extern int errno;

struct hostent * host;
struct sockaddr_in server;

struct decipherSocket{
	int port;
	int read;
	int error;
	size_t nbytes;
	char * buffer;
} decipherSocket;

int client_mode; 
int sd;
socklen_t serv_addrLen;
char ip[100];
char * host_name;

int netserverinit(char * , int );
int netopen(const char *, int);
ssize_t netread(int, void *, size_t);
ssize_t netwrite(int, const void *, size_t);
int netclose(int);
#endif

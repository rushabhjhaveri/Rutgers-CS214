#ifndef _LIBNETFILES_H_
#define _LIBNETFILES_H_

#define PORT 9696
#define MAX_BUFFER_SIZE 1024
#define MAX_SOCKETS 5
#define INVALID_FILE_MODE 500

#define UNRESTRICTED 0
#define EXCLUSIVE 1
#define TRANSACTION 2

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

#endif

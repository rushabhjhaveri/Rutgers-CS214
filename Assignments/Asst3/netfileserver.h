#ifndef _NETFILESERVER_H_
#define _NETFILESERVER_H_

#define PORT 9696
#define MAX_CLIENTS 10
#define MAX_SOCKETS 10
#define MAX_STREAMS_PER_CLIENT 5
#define TIMEOUT 3

extern int errno;
extern int h_errno;

pthread_mutex_t lock;
pthread_mutex_t list;
pthread_mutex_t sockets;
pthread_mutex_t conflict;
pthread_mutex_t conflict2;

struct fdlist{
	char * filename;
	int nnode;
	int mode;
	int fd;
	int filemode;
	struct fdlist * next;
}fdlist;

struct clientlist{
	int client;
	int mode;
	int flag;
	int fd[2];
	struct timeval start;
	struct clientlist * next;
}clientlist;

struct filelist{
	char * filename;
	int nnode;
	struct filelist * next;
	struct clientlist * list;
}filelist;

struct socketlist{
	int port;
	int client;
	//int part /* Wat iz dis  */
	int clientsocket;
	int fd;
	int nread;
	size_t nbytes;
	char * buffer;
	socklen_t serverLen;
	struct sockaddr_in server;
}socketlist;


struct socketlist socketL[MAX_SOCKETS];

struct fdlist * fdL;
struct filelist * fL;

#endif

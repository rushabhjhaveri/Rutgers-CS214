#ifndef _NETFILESERVER_H_
#define _NETFILESERVER_H_

#define PORT 9697
#define MAX_CLIENTS 10
#define MAX_SOCKETS 10
#define MAX_STREAMS_PER_CLIENT 5
#define TIMEOUT 3

// for Extension A //
#define UNRESTRICTED 0
#define EXCLUSIVE 1
#define TRANSACTION 2
#define PERMS_ERROR 100000


extern int errno;
extern int h_errno;

pthread_mutex_t lock;
pthread_mutex_t list;
pthread_mutex_t sockets;
pthread_mutex_t conflict;
pthread_mutex_t conflict2;


// typedefs
// for file mode and permissions check
typedef enum {
	         U_R = 0, U_W =1,U_RW=2, E_R = 3, E_W=4, E_RW=5, T_R=6, T_W=7, T_RW=8
} fileModePerms;

typedef enum {
	   false=0, true=1
} bool;


/*
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
	fileModePerms fmp;	
	struct timeval start;
	struct clientlist * next;
}clientlist;
*/


struct clientlist{
	int client;
	int mode;
	int flag;
	int fd;
	fileModePerms fmp;	
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
	int part; 
	int clientsocket;
	int fd;
	int nread;
	size_t nbytes;
	char * buffer;
	socklen_t serverLen;
	struct sockaddr_in server;
}socketlist;


struct socketlist socketL[MAX_SOCKETS];

//struct fdlist * fdL;
struct filelist * fL;

int isFileModePermsOKtoOpen(fileModePerms, fileModePerms);
fileModePerms  mapFileModePerms(int, int);
#endif

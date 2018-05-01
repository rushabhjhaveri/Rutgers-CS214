#include "libnetfiles.h"


int main (){
	char * hostname = "localhost";
	netserverinit(hostname,0);
	
	char * filename = "file.txt";
	int fdd = netopen(filename,O_RDWR);
	
	FILE * file = fopen(filename, "r");
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	printf("SIZE: %d\n",size);
	
	
	char * filename2 = "file2.txt";
	int fdd2 = netopen(filename2,O_RDWR);
	
        char ans ;
	printf("Press any key to continue \n");
	scanf("%c", &ans);
	
	char * txt = calloc(1,4096);
	printf("Read: %zd\n",netread(fdd,txt,4096));
	printf("In client, txt: %s\n", txt);
	
	printf("Write: %zd\n",netwrite(fdd2,txt,size));
	/*
	printf("Press any key to continue \n");
	scanf("%c", &ans);
	
	char * filename3 = "file3.txt";
	int fdd3 = netopen(filename3, 0);
	FILE * f = fopen(filename3, "r");
	fseek(f, 0L, SEEK_END);
	size = ftell(f);
	char * t = calloc(1, size);
	printf("Read: %zd\n", netread(fdd3, t, size));
	printf("[%d] %s\n",errno,strerror(errno));
	*/
	int fcc =  netclose(fdd);
	printf("Netclose %d\n[%d] %s\n",fcc,errno,strerror(errno));
	//free(txt);
	
	return 0;
}

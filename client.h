#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#define	 MY_PORT  2222

struct username
{
	uint8_t length;
	char* name;
	int fd;
};


char *inputMessage(FILE* fp, size_t size);
void getStringFromRecv(int s, char * str, int len);
void receiveMessage(int s, int flag);


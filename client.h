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

// #define	 MY_PORT  2222

typedef int bool;
#define true 1
#define false 0

struct username
{
	uint8_t length;
	char* name;
};

// struct username user;
// struct username * users;
// int numberOfUsers = 0;

char *inputMessage(FILE* fp, size_t size);

bool receivedHandshake(int s);

void getStringFromRecv(int s, char * str, uint8_t len);
void receiveMessage(int s, uint8_t flag);

// void getCurrentUserList(int s, struct username * users, int numberOfUsers);


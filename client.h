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

bool receivedHandshake(int s);

void getStringFromRecv(int s, char * str, int len);
void receiveMessage(int s, uint8_t flag, struct username * users, uint16_t* numberOfUsers); // expecting length string (msglen msg is flag is 0x00)

void addUserName(struct username * users, uint16_t* numberOfUsers, char* name, int len);
void printCurrentUserList(struct username * users, uint16_t numberOfUsers);


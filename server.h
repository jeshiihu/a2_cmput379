#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>


#define	MY_PORT	2222

typedef int bool;
#define true 1
#define false 0

struct username
{
	int length;
	char* name;
};

void printUsers(struct username * users, int numberOfUsers);
void addUserName(struct username * users, int size, char* name, int nameLen);

void sendInitialHandshake(int sock);
void sendNumberOfUsers(int sock, int numUsers);

int getUsernameLength(int sock);
bool isUniqueUsername(struct username * users, int size, char* newUser);
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
	uint8_t length;
	char* name;
	int fd;
};

void printUsers(struct username * users, uint16_t numberOfUsers);
void addUserName(struct username * users, uint16_t size, char* name, int nameLen, int fd);
struct username* deleteUser(struct username * users, uint16_t size, int fd);

void sendInitialHandshake(int sock);
void sendNumberOfUsers(int sock, uint16_t numUsers);
void sendAllUserNames(int listener, struct username* users, uint16_t numberOfUsers);

int getUsernameLength(int sock);
uint16_t getMessageLength(int sock);
bool isUniqueUsername(struct username * users, uint16_t size, char* newUser);
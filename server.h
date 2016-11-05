#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

typedef int bool;
#define true 1
#define false 0

static FILE *fp;

struct username
{
	uint8_t length;
	char* name;
	int fd;
};

void sendString(int s, char* str, int len);

void printUsers(struct username * users, uint16_t numberOfUsers);
void addUserName(struct username * users, uint16_t size, char* name, uint8_t nameLen, int fd);
struct username* deleteUser(struct username * users, uint16_t size, int fd);

void sendInitialHandshake(int sock);
void sendNumberOfUsers(int sock, uint16_t numUsers);
void sendAllUserNames(int listener, struct username* users, uint16_t numberOfUsers);
void sendMessageToAllUsers(struct username * users, uint16_t numberOfUsers, char* sendingUser, uint8_t sendingUserLen, int messageLength, char * message);


int getUsernameLength(int sock);
uint16_t getMessageLength(int sock);
bool isUniqueUsername(struct username * users, uint16_t size, char* newUser);

void receiveString(int s, char * str, int len);
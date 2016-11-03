
#include "client.h"
/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.				 
 --------------------------------------------------------------------- */
char *inputMessage(FILE* fp, size_t size)
{
	char *str;
	int ch;
	size_t len = 0;
	str = realloc(NULL, size * sizeof(*str));
	if(!str) {
		return str;
	} 
	while (EOF != (ch=fgetc(fp)) && ch != '\n') {
		str[len++]=ch;
		if (len==size) {
			str = realloc(str, sizeof(*str)*(size+=size));
			if (!str) {
				return str;
			}
		}
	}
	str[len++] = '\0';

	return realloc(str, sizeof(*str)*len);
}

bool receivedHandshake(int s)
{
	printf("inside handshake\n");
	uint8_t * handshakeBuf = malloc(2 * sizeof(uint8_t));
	int bytes;
	bytes = recv(s, handshakeBuf, 2* sizeof(uint8_t),0);

	uint8_t first = 0xcf;
	uint8_t second = 0xaf;
	printf("hand0: %x, hand1: %x\n", handshakeBuf[0], handshakeBuf[1]);
	if(handshakeBuf[0] == first && handshakeBuf[1] == second)
	{
		return true;
	}

	return false;
}

void getStringFromRecv(int s, char * str, int len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		char c;
		
		while(1)
		{
			int bytes = recv(s, &c, sizeof(c), 0);
			if(bytes == 1 && (strlen(&c) != 0))
			{
				str[i] = c;
				break;
			}
		}
	}

	str[len] = '\0';
}

void receiveMessage(int s, uint8_t flag, struct username * users, uint16_t* numberOfUsers) // expecting length string (msglen msg is flag is 0x00)
{
	uint8_t msg = 0x00;
	uint8_t join = 0x01;
	uint8_t leave = 0x02;

	uint8_t userLen;
	int bytes;
	if ((bytes = recv(s, &userLen, sizeof(userLen), 0)) < 0) // get the first length
	{
		printf("Error in reading from Server\n");
		return;
	}

	char name[userLen + 1];
	// bytes = recv(s, &name, sizeof(name), 0);
	// name[userLen] = '\0';
	getStringFromRecv(s, name, userLen);

	if(flag == msg) // regular message
	{
		uint16_t msgLength;
		while(1)
		{
			bytes = recv(s, &msgLength, sizeof(msgLength), 0);
			msgLength = ntohs(msgLength);

			if(bytes == 2 && msgLength > 0)
			{
				break;
			}
		}

		char msg[msgLength + 1];
		getStringFromRecv(s, msg, msgLength);
		printf("User %s: %s\n", name, msg);
	}
	else if(flag == join)
	{
		printf("User %s: joined the server!\n", name);
		addUserName(users, numberOfUsers, name, userLen +1);
	}
	else if(flag == leave)
		printf("User %s: disconnected from server.\n", name);
}

void addUserName(struct username * users, uint16_t* numberOfUsers, char* name, int len)
{
	// printCurrentUserList(users, *numberOfUsers);

	int index = (*numberOfUsers);
	if(*numberOfUsers == 0)
		users = malloc(1 * sizeof(struct username));
	else
		users = realloc(users, (*numberOfUsers) * sizeof(struct username));

	users[index].length = len;
	users[index].name = malloc(len * sizeof(char));
	strcpy(users[index].name, name);

	*numberOfUsers = *numberOfUsers + 1;

}


void populateUserList(int s, struct username * users, uint16_t numberOfUsers)
{
	printf("Number of Users:  %d\n", ntohs(numberOfUsers));
	users = realloc(users, numberOfUsers * sizeof(struct username));

	int i;
	for(i = 0; i < numberOfUsers; i++)
	{
		uint8_t len;
		int bytes;
		while(1) 
		{ 
			bytes = recv(s, &len, sizeof(len), 0);
			if(bytes == 1) 
				break;
		}

		users[i].length = len;
	}
}

void printCurrentUserList(struct username * users, uint16_t numberOfUsers)
{
	printf("Number of users: %d\n", numberOfUsers);
	
	int i;
	for(i = 0; i < numberOfUsers; i++)
		printf("users[%d] = %s\n", i, users[i].name);
}

void recvAllCurrentUsers(int s, uint16_t numberOfUsers, struct username * users)
{
	printf("Number of current users: %d\n", numberOfUsers);
	if(numberOfUsers > 0)
		users = realloc(users, numberOfUsers*sizeof(struct username));

	int i;
	for(i = 0; i < numberOfUsers; i++)
	{
		uint8_t len;
		while(1)
		{
			int bytes = recv(s, &len, sizeof(len), 0);
			if(bytes == 1 && len > 0)
				break;
		}

		char name[len+1];
		getStringFromRecv(s, name, len);

		users[i].length = len;
		users[i].name = malloc(len * sizeof(char));
		strcpy(users[i].name, name);

		printf("user[%d]: %s\n", i, name);
	}

	printf("\n");
}


int main(int argc, char** argv)
{	
	if(argc != 4)
	{
		printf("Invalid client input. Should be formatted as: hostname portnumber username\n");
		return 0; 
	}

	// struct username user;
	// struct username * users = malloc(1 * sizeof(user));
	// uint16_t numberOfUsers = 0;

	struct threadData data;
	data.numberOfUsers = 0;
	data.users = malloc(1 * sizeof(struct username));

	// get the client inputs!
	char hostname[strlen(argv[1])];
	strcpy(hostname, argv[1]);
	
	char portnumberStr[strlen(argv[2])];
	strcpy(portnumberStr, argv[2]);
	uint16_t port = (uint16_t)atoi(portnumberStr);
	
	char username[strlen(argv[3])];
	strcpy(username, argv[3]);

	// int	s, number;

	struct	sockaddr_in	server;

	struct	hostent	*host;
	host = gethostbyname(hostname);

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}

	while (1) {
		data.s = socket (AF_INET, SOCK_STREAM, 0);

		if (data.s < 0) {
			perror ("Client: cannot open socket");
			exit (1);
		}

		bzero (&server, sizeof (server));
		bcopy (host->h_addr, & (server.sin_addr), host->h_length);
		server.sin_family = host->h_addrtype;
		server.sin_port = htons(port);

		if (connect (data.s, (struct sockaddr*) & server, sizeof (server))) {
			perror ("Client: cannot connect to server");
			exit (1);
		}

		// if flag is 0 then it works exactly like read();
		if(receivedHandshake(data.s))
		{
			printf("successful handshake!\n");
			recv(data.s, &(data.numberOfUsers), sizeof(uint16_t), 0);
			data.numberOfUsers = ntohs(data.numberOfUsers);

			recvAllCurrentUsers(data.s, data.numberOfUsers, data.users);
			printf("received all users, double check with print\n");
			printCurrentUserList(data.users, data.numberOfUsers);
			printf("\n");

			int len = (int)strlen(username);
			send(data.s, &len, sizeof(len), 0); // send username length
			send(data.s, username, sizeof(username), 0);


			pthread_t threadInput;
			if(pthread_create(&threadInput, NULL, listenToInput, &data))
			{
				perror("Thread listen to input failed.");
				exit(1);
			}

			pthread_t threadServer;
			if(pthread_create(&threadServer, NULL, listenToServer, &(data.s)))
			{
				perror("Thread listen to server failed.");
				exit(1);
			}

			if(pthread_join(threadInput, NULL)) 
			{
				perror("Error joining threadInput.");
				exit(1);
			}
			if(pthread_join(threadServer, NULL)) 
			{
				perror("Error joining threadServer.");
				exit(1);
			}
		}
		close(data.s);
	}

	return 0;
}

void* listenToInput(void* param)
{
	while(1)
	{	
		int* s = (int*)param;
		char *message;
		uint16_t messageLength;
		
		message = inputMessage(stdin, sizeof(uint16_t));
		if(strlen(message) > 0)
		{
			messageLength = strlen(message);

			int messageLengthInNBO = htons(messageLength);
			int bytes = send(*s, &messageLengthInNBO, sizeof(messageLengthInNBO),0);
			if(bytes < 0)
			{
				printf("Error: Closing connection\n");
				close(*s);
				exit(1);
			}

			char sentMessage[messageLength];
			strcpy(sentMessage, message);

			bytes = send(*s, sentMessage, sizeof(sentMessage), 0);
			if(bytes < 0)
			{
				printf("Error: Closing connection\n");
				close(*s);
				exit(1);
			}

			free(message);
		}
	}

	return NULL;
}

void* listenToServer(void* param)
{
	while(1)
	{
		struct threadData* data = (struct threadData*)param;
		uint8_t messageFlag;
		int bytes = recv(data->s, &messageFlag, sizeof(messageFlag), 0);
		if((bytes == 1 && (messageFlag == (uint8_t)(0x01) || messageFlag == (uint8_t)(0x02))) ||
			(bytes == 2 && messageFlag == (uint8_t)(0x00))) // received the flag
		{	
			receiveMessage(data->s, messageFlag, data->users, &(data->numberOfUsers));
		}
		else
		{
			printf("Error: Closing connection\n");
			close(data->s);
			exit(1);
		}
	}

	return NULL;
}

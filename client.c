
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
	uint8_t * handshakeBuf = malloc(2 * sizeof(uint8_t));
	int bytes;
	bytes = recv(s, handshakeBuf, 2* sizeof(uint8_t),0);

	uint8_t first = 0xcf;
	uint8_t second = 0xaf;
	if(handshakeBuf[0] == first && handshakeBuf[1] == second)
		return true;

	return false;
}

void getStringFromRecv(int s, char * str, uint8_t len)
{
	int currentBytesRead = 0;
	while(currentBytesRead < len)
	{
		char c;
		int bytesReceived = recv(s, &c, sizeof(c), 0);

		if (strlen(&c) != 0) {
			str[currentBytesRead] = c;
			currentBytesRead = currentBytesRead + bytesReceived;
		}
	}

	str[len] = '\0';
}

void receiveMessage(int s, uint8_t flag, struct username * users, uint16_t* numberOfUsers) // expecting length string (msglen msg is flag is 0x00)
{
	printf("the flag is: %x \n", flag);
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
	getStringFromRecv(s, name, userLen);

	if(flag == msg) // regular message
	{
		uint16_t msgLenth;
		if((bytes = recv(s, &msgLenth, sizeof(msgLenth), 0)) > 0)
		{
			uint16_t msgLenth = ntohs(msgLenth);
			char msg[msgLenth + 1];
			getStringFromRecv(s, msg, msgLenth);
			printf("User %s: %s\n", name, msg);
		}
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
	*numberOfUsers = *numberOfUsers + 1;
	int index = (*numberOfUsers) - 1;
	users = realloc(users, (*numberOfUsers) * sizeof(struct username));

	users[index].length = len;
	users[index].name = malloc(len * sizeof(char));
	strcpy(users[index].name, name);
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

void recvAllCurrentUsers(int s, uint16_t numberOfUsers)
{
	printf("Number of current users: %d\n", numberOfUsers);
}


int main(int argc, char** argv)
{	
	if(argc != 4)
	{
		printf("Invalid client input. Should be formatted as: hostname portnumber username\n");
		return 0; 
	}

	struct username user;
	struct username * users = malloc(1 * sizeof(user));
	uint16_t numberOfUsers = 0;

	// get the client inputs!
	char hostname[strlen(argv[1])];
	strcpy(hostname, argv[1]);
	
	char portnumberStr[strlen(argv[2])];
	strcpy(portnumberStr, argv[2]);
	uint16_t port = (uint16_t)atoi(portnumberStr);
	
	char username[strlen(argv[3])];
	strcpy(username, argv[3]);

	int	s, number;

	struct	sockaddr_in	server;

	struct	hostent	*host;
	host = gethostbyname(hostname);

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}

	while (1) {
		s = socket (AF_INET, SOCK_STREAM, 0);

		if (s < 0) {
			perror ("Client: cannot open socket");
			exit (1);
		}

		bzero (&server, sizeof (server));
		bcopy (host->h_addr, & (server.sin_addr), host->h_length);
		server.sin_family = host->h_addrtype;
		server.sin_port = htons(port);

		if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
			perror ("Client: cannot connect to server");
			exit (1);
		}

		// if flag is 0 then it works exactly like read();
		if(receivedHandshake(s))
		{
			recv(s, &numberOfUsers, sizeof(numberOfUsers), 0);
			numberOfUsers = ntohs(numberOfUsers);
			recvAllCurrentUsers(s, numberOfUsers);
			
			int len = (int)strlen(username);
			send(s, &len, sizeof(len), 0); // send username length
			send(s, username, sizeof(username), 0);
			// printf("Name: %s\n", username);

			pid_t forkID = fork();
			while(1)
			{
				char *message;
				uint16_t messageLength;
				
				if(forkID == 0) // if parent then always wait for client input message
				{
					if(message = inputMessage(stdin, sizeof(uint16_t)))
					{
						messageLength = strlen(message);

						int messageLengthInNBO = htons(messageLength);
						send(s, &messageLengthInNBO, sizeof(messageLengthInNBO),0);
						
						char sentMessage[messageLength];
						strcpy(sentMessage, message);

						send(s, sentMessage, sizeof(sentMessage), 0);
	
						free(message);
					}
				}
				else // child process wants to always listen to the incoming messages
				{
					uint8_t messageFlag;
					if(recv(s, &messageFlag, sizeof(messageFlag), 0) > 0) // received the flag
					{	
						receiveMessage(s, messageFlag, users, &numberOfUsers);
					}
				}
			}
		}

		sleep(1);
		close(s);
	}

	return 0;
}

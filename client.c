
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

// // COMMON FUNCTION MOVE TO A COMMON FILE LATER!!!
// void addUserName(char* name, int nameLen)
// {
// 	users[numberOfUsers - 1].length = nameLen;
// 	users[numberOfUsers - 1].name = malloc(nameLen * sizeof(char));
// 	strcpy(users[numberOfUsers - 1].name, name);
// }

void receiveMessage(int s, uint8_t flag) // expecting length string (msglen msg is flag is 0x00)
{
	uint8_t userLen;
	int bytes;
	if ((bytes = recv(s, &userLen, sizeof(userLen), 0)) < 0) // get the first length
	{
		printf("Error in reading from Server\n");
		return;
	}

	// userLen = ntohs(userLen);
	char name[userLen + 1];
	getStringFromRecv(s, name, userLen);

	// if(flag == ((uint8_t)0x00)) // regular message
	// {
	// 	userLen = ntohs(userLen);
	// 	char name[userLen + 1];
	// 	getStringFromRecv(s, name, userLen);
	// 	printf("User %s of length: %d \n", name, userLen);
	// }

	printf("the flag is: %d \n", flag);

	if(flag == ((uint8_t)0x00)) // regular message
	{
		uint16_t msgLenth;
		if((bytes = recv(s, &msgLenth, sizeof(msgLenth), 0)) > 0)
		{
			int msgLenth = ntohs(msgLenth);
			char msg[msgLenth + 1];
			getStringFromRecv(s, msg, msgLenth);
			printf("User %s: %s\n", name, msg);
		}
	}
	else if(flag == ((uint8_t)0x01))
	{
		printf("User %s: joined the server!\n", name);
		// addUserName(name, userLen +1);
		// printCurrentUserList();
	}
	else if(flag == ((uint8_t)0x02))
		printf("User %s: disconnected from server.\n", name);
}



// void populateUserList(int s, struct username * users, int size, int numOfUsersToAdd)
// {
// 	int i;
// 	for(i = 0; i < numberOfUsers; i++)
// 	{
// 		// addUserName(users, size,)
// 		;
// 	}
// }

// void printCurrentUserList()
// {
// 	printf("Number of users: %d\n", numberOfUsers);
	
// 	int i;
// 	for(i = 0; i < numberOfUsers; i++)
// 		printf("users[%d] = %s\n", i, users[i].name);
// }



int main(int argc, char** argv)
{	
	if(argc != 4)
	{
		printf("Invalid client input. Should be formatted as: hostname portnumber username\n");
		return 0; 
	}

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
			uint16_t numberOfUsers;
			recv(s, &numberOfUsers, sizeof(numberOfUsers),0);
			printf("Number of Users:  %d\n", ntohs(numberOfUsers));
			// getCurrentUserList(s, users, numberOfUsers);
			
			int len = (int)strlen(username);
			send(s, &len, sizeof(len), 0); // send username length
			send(s, username, sizeof(username), 0);
			printf("Name: %s\n", username);


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
					int messageFlag;
					if(recv(s, &messageFlag, sizeof(messageFlag, 0), 0) > 0) // received the flag
						receiveMessage(s, messageFlag);
				}
			}
		}

		close(s);
	}

	return 0;
}

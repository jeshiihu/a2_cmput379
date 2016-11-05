
#include "client.h"
/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.				 
 --------------------------------------------------------------------- */
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

void getStringFromRecv(int s, char * str, int len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		char c;
		
		while(1)
		{
			int bytes = recv(s, &c, sizeof(c), 0);
			if(bytes == 1)
			{
				str[i] = c;
				break;
			}
			else if(bytes < 0)
			{
				perror("Trying to receive a string failed.");
				exit(1);
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
		perror("Error in reading from Server");
		exit(1);
	}

	char name[userLen + 1];
	getStringFromRecv(s, name, userLen);

	if(flag == msg) // regular message
	{
		uint16_t msgLength;
		while(1) // need to recieve the message length from the server
		{
			bytes = recv(s, &msgLength, sizeof(msgLength), 0);
			msgLength = ntohs(msgLength);

			if(bytes == 2 && msgLength > 0)
			{
				break;
			}
			else if(bytes < 0)
			{
				perror("Error receiving message length from server.");
				exit(1);
			}
		}

		char msg[msgLength + 1];
		getStringFromRecv(s, msg, msgLength); //get the message from the server
		printf("User %s: %s\n", name, msg);
	}
	else if(flag == join)
	{
		printf("User %s: joined the server!\n", name);
		*numberOfUsers = *numberOfUsers + 1;
		addUserName(&users, *numberOfUsers, name, userLen);
	}
	else if(flag == leave)
	{
		printf("User %s: disconnected from server.\n", name);
		deleteUsername(&users, *numberOfUsers, name);
		*numberOfUsers = *numberOfUsers - 1;
	}
}

void addUserName(struct username ** users, uint16_t numberOfUsers, char* name, int len)
{
	(*users)[numberOfUsers - 1].length = len;
	(*users)[numberOfUsers - 1].name = malloc((len+1) * sizeof(char));
	strcpy((*users)[numberOfUsers - 1].name, name);
}

void deleteUsername(struct username ** users, uint16_t numberOfUsers, char* name) //deletes users in an ugly fashion of shifting other elements up and freeing space
{
	// printf("in delete\n");
	int index;
	bool foundUser = false;
	for(index = 0; index < numberOfUsers; index++)
	{
		if(strcmp((*users)[index].name, name) == 0) // found the user!
		{
			foundUser = true;
			break;
		}
	}

	if(foundUser == false)
	{
		printf("Could not find user %s to delete", name);
		exit(1);
	}

	struct username user; // used to get size
	struct username* tempUsers = malloc((numberOfUsers - 1) * sizeof(user)); //create array of size one less than it was

	if(index == 0)	// want to remove the first element, so copy every thing before this element
	{
		memcpy(tempUsers, (*users)+1, (numberOfUsers - 1) * sizeof(user));
	}
	else if(index == (numberOfUsers-1)) // want to remove the last element so copy everything until the last
	{
		memcpy(tempUsers, (*users), (numberOfUsers - 1) * sizeof(user));
	}
	else // want to remove an element in between, copy before and then copy after
	{
		memcpy(tempUsers, (*users), (index) * sizeof(user));
		memcpy(tempUsers+index, (*users)+index+1, (numberOfUsers-index-1) * sizeof(user));
	}

	*users = realloc((*users), (numberOfUsers-1)*sizeof(user));
	*users = tempUsers;
	// printCurrentUserList(*users, numberOfUsers-1);
	free(tempUsers);
}

void printCurrentUserList(struct username * users, int numberOfUsers)
{
	printf("Number of users: %d\n", numberOfUsers);
	int i;
	for(i = 0; i < numberOfUsers; i++)
	{
		printf("users[%d] = %s\n", i, users[i].name);
	}
}

void recvAllCurrentUsers(int s, uint16_t numberOfUsers, struct username ** users)
{
	printf("Receiving list of current users: %d users\n", numberOfUsers);
	if(numberOfUsers > 1) // need to add this to the user list for each client
		*users = realloc((*users), numberOfUsers*sizeof(struct username));

	int i;
	for(i = 0; i < numberOfUsers; i++)
	{
		uint8_t len;
		while(1)
		{
			int bytes = recv(s, &len, sizeof(len), 0);
			if(bytes == 1 && len > 0)
				break;
			if(bytes < 0)
			{
				perror("Error: Closing connection\n");
				close(s);
				exit(1);
			}
		}

		char name[len+1];
		getStringFromRecv(s, name, len);
		printf("user[%d]: %s\n", i, name);

		// add to the users!
		(*users)[i].length = len;
		(*users)[i].name = malloc((len+1)*sizeof(char));
		strcpy((*users)[i].name, name);
	}
}

void sendStringClient(int s, char* str, int len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		while(1)
		{
			int bytes = send(s, &str[i], sizeof(char), 0);
			if(bytes == 1)
			{
				// printf("%c\n", str[i]);
				break;
			}
			if(bytes < 0)
			{
				perror("Error: Trying to send string\n");
				close(s);
				exit(1);
			}
		}
	}
}

int main(int argc, char** argv)
{	
	if(argc != 4)
	{
		printf("Invalid client input. Should be formatted as: ./server379 <hostname> <portnumber> <username>\n");
		return 0; 
	}

	struct username user;
	struct username * users;
	users = malloc(1 * sizeof(user));
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
		// printf("while\n");
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

		fd_set master, read_fds; // master file descriptor list, and temp file descriptor list
		FD_ZERO(&master); // clear the master and temp set
		FD_ZERO(&read_fds);

		FD_SET(0, &master); 
		FD_SET(s, &master); // add the socket to the master set

		int fdmax = s; 
		// if flag is 0 then it works exactly like read();
		if(receivedHandshake(s))
		{
			int byte = recv(s, &numberOfUsers, sizeof(numberOfUsers), 0);
			if(byte < 0)
			{
				perror("Error: Closing connection\n");
				close(s);
				exit(1);
			}

			numberOfUsers = ntohs(numberOfUsers);
			recvAllCurrentUsers(s, numberOfUsers, &users);

			uint8_t len = (uint8_t)strlen(username);
			int bytes = send(s, &len, sizeof(uint8_t), 0); // send username length
			sendStringClient(s, username, len);

			pthread_t dummy;
			if(pthread_create(&dummy, NULL, sendDummy, &s))
			{
				printf("Failed to create thread\n");
			}

			while(1)
			{
				read_fds = master; // copy it
				if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
				{
					perror("Server: cannot select file descriptor");
					exit(1);
				}

				int i;
				for(i = 0; i <= fdmax; i++) // this loops through the file descriptors
				{
					if(FD_ISSET(i, &read_fds)) // we got one!!
					{
						if(i == 0) // not our socket
						{
							char message[256];
							uint16_t messageLength;

							fgets(message, sizeof(message), stdin);
							if(strlen(message) == 0)
								break;

							message[strcspn(message, "\n")] = '\0';
							messageLength = strlen(message);
							char sentMessage[messageLength];
							strcpy(sentMessage, message);

							char list[] = "-listUsers";
							if(strcmp(sentMessage, list) == 0) // call to list current users
							{
								printf("\n");
								printCurrentUserList(users, numberOfUsers);
								continue;
							}

							uint16_t messageLengthInNBO = htons(messageLength);
							int bytes = send(s, &messageLengthInNBO, sizeof(messageLengthInNBO),0);
							if(bytes < 0)
							{
								perror("Error: Closing connection\n");
								close(s);
								exit(1);
							}

							sendStringClient(s, sentMessage, messageLength);						
						}
						else
						{
							uint8_t messageFlag;
							int bytes = recv(s, &messageFlag, sizeof(messageFlag), 0);
							if(bytes == 1) // received the flag
							{	
								receiveMessage(s, messageFlag, users, &numberOfUsers);
							}
							else if(bytes <= 0)
							{
								perror("Server disconnected.");
								close(s);
								exit(1);
							}
						}	
					}
				}
			}
		}
		close(s);
	}

	return 0;
}


void* sendDummy(void* param)
{
	while(1)
	{
		int* sock = (int*)param;
		sleep(30);
		uint16_t dummyLength = 0;
		send(*sock, &dummyLength, sizeof(uint16_t), 0);
		// printf("sent dummy\n");
	}

	return NULL;
}







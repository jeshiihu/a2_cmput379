
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
		}

		char msg[msgLength + 1];
		getStringFromRecv(s, msg, msgLength); //get the message from the server
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
	int index = (*numberOfUsers);

	int newSize = ((*numberOfUsers)+1);
	users = realloc(users, newSize * sizeof(struct username));

	users[index].length = len;
	users[index].name = malloc((len+1) * sizeof(char));
	strcpy(users[index].name, name);
	*numberOfUsers = *numberOfUsers + 1;
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

void recvAllCurrentUsers(int s, uint16_t numberOfUsers)
{
	printf("Number of current users: %d\n", numberOfUsers);

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
		printf("len user:%d\n", len);
		char name[len+1];
		getStringFromRecv(s, name, len);
		printf("user[%d]: %s\n", i, name);
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
				break;
			}
			if(bytes < 0)
			{
				perror("Error: Closing connection\n");
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
			recvAllCurrentUsers(s, numberOfUsers);
			
			uint8_t len = (uint8_t)strlen(username);
			int bytes = send(s, &len, sizeof(uint8_t), 0); // send username length
			sendStringClient(s, username, len);

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

							messageLength = strlen(message);

							uint16_t messageLengthInNBO = htons(messageLength);
							int bytes = send(s, &messageLengthInNBO, sizeof(messageLengthInNBO),0);
							if(bytes < 0)
							{
								perror("Error: Closing connection\n");
								close(s);
								exit(1);
							}

							char sentMessage[messageLength];
							strcpy(sentMessage, message);

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
							else if(bytes < 0)
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

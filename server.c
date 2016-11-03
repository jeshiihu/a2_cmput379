
#include "server.h"

/* ---------------------------------------------------------------------
 This	is  a sample server which opens a stream socket and then awaits
 requests coming from client processes. In response for a request, the
 server sends an integer number  such	 that  different  processes  get
 distinct numbers. The server and the clients may run on different ma-
 chines.
 --------------------------------------------------------------------- */
void printUsers(struct username * users, uint16_t numberOfUsers)
{
	printf("Number of users: %d\n", numberOfUsers);
	
	int i;
	for(i = 0; i < numberOfUsers; i++)
		printf("users[%d] = %s\n", i, users[i].name);
}

void addUserName(struct username * users, uint16_t numberOfUsers, char* name, uint8_t nameLen, int fd)
{
	users[numberOfUsers - 1].length = nameLen;
	users[numberOfUsers - 1].fd = fd;

	users[numberOfUsers - 1].name = malloc(nameLen * sizeof(char));
	strcpy(users[numberOfUsers - 1].name, name);
}

int getUserIndex(struct username * users, uint16_t numberOfUsers, int fd)
{
	int index;
 	for(index = 0; index < numberOfUsers; index++)
	{
	 	//printf("comparing fd to be checked %d to passed fd %d \n",users[index].fd,fd );
	 	if(users[index].fd == fd) // found the user
	 		break;
	}

	return index;
}

struct username* deleteUser(struct username * users, uint16_t numberOfUsers, int fd) //deletes users in an ugly fashion of shifting other elements up and freeing space
{
	printf("in deleteUser\n");
	int index = getUserIndex(users, numberOfUsers, fd);

	struct username user; // used to get size
	struct username* tempUsers = malloc((numberOfUsers - 1) * sizeof(user)); //create array of size one less than it was

	if(index == 0)	// want to remove the first element, so copy every thing before this element
	{
		memcpy(tempUsers, users+1, (numberOfUsers - 1) * sizeof(user));
	}
	else if(index == (numberOfUsers-1)) // want to remove the last element so copy everything until the last
	{
		memcpy(tempUsers, users, (numberOfUsers - 1) * sizeof(user));
	}
	else // want to remove an element in between, copy before and then copy after
	{
		memcpy(tempUsers, users, (index) * sizeof(user));
		memcpy(tempUsers+index, users+index+1, (numberOfUsers-index-1) * sizeof(user));
	}

	free(users);

	return tempUsers;
}

void sendInitialHandshake(int listener)
{
	uint8_t * handshakeArray = malloc(2 * sizeof(uint8_t));
	handshakeArray[0] = 0xcf;
	handshakeArray[1] = 0xaf;
	int bytesSentFirst;
	bytesSentFirst = send(listener, handshakeArray, 2 * sizeof(uint8_t), 0);
}

void sendNumberOfUsers(int listener, uint16_t numUsers)
{
	printf("sending number of users...\n" );

	uint16_t num = htons(numUsers);
	int bytes = send (listener, &num, sizeof(num), 0);
	printf("%d bytes sent: number of users is %d\n", bytes, numUsers);
}

void sendAllUserNames(int listener, struct username* users, uint16_t numberOfUsers)
{
	int i;
	for(i = 0; i < numberOfUsers; i++)
	{
		uint8_t len;
		int bytes;
		while(1)
		{
			len = users[i].length;
			bytes = send(listener, &len, sizeof(len), 0);
			if(bytes == 1)
				break;
		} 

		printf("%d bytes sent, len of user: %d\n", bytes, users[i].length);

		int expectedBytes = sizeof(users[i].name);
		sendString(listener, users[i].name, len);

		// bytes = send(listener, users[i].name, sizeof(users[i].name), 0);
		printf("Expected %d bytes, %d bytes sent, : %s\n", expectedBytes, bytes, users[i].name);
	}
}

int getUsernameLength(int listener)
{
	int i;
	recv(listener, &i, sizeof(i), 0);

	return i;
}

uint16_t getMessageLength(int listener)
{
	uint16_t i;
	recv(listener, &i, sizeof(i), 0);

	return i;
}

bool isUniqueUsername(struct username * users, uint16_t numberOfUsers, char* newUser)
{
	int i;
	for (i = 0; i < numberOfUsers; i++) 
	{
		if (strcmp(users[i].name, newUser) == 0) // two names match so invalid username since not unqiue
			return false;
	}

	return true;
}

int getListener()
{
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}

	return listener;
}

void sendString(int s, char* str, int len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		while(1)
		{
			int bytes = send(s, &str[i], sizeof(str[i]), 0);
			if(bytes == 1)
				break;
		}
	}
}

void sendMessageToAllUsers(struct username * users, uint16_t numberOfUsers, char* sendingUser, uint8_t sendingUserLen, int messageLength, char * message)
{
	int index;
	for(index = 0; index < numberOfUsers; index++) 
	{
		int fd = users[index].fd;

		uint8_t flag = 0x00; // send regular message flag
		send(fd, &flag, sizeof(flag), 0);

		// send username!
		int byte = send(fd, &sendingUserLen, sizeof(sendingUserLen), 0);
		sendString(fd, sendingUser, sendingUserLen);

		uint16_t messageLengthInNBO = htons(messageLength);
		int sendByte = send(fd, &messageLengthInNBO, sizeof(messageLengthInNBO), 0);
		
		sendString(fd, message, messageLength);

		printf("sent message: %s to fd: %d \n",message, fd);
	}
}

void sendUsernameAndLength(int fd, struct username * users, uint16_t numberOfUsers, char * username, uint8_t usernameLength)
{
	uint8_t usernameLengthInNBO = usernameLength;
	int sendByte = send(fd, &usernameLengthInNBO, sizeof(usernameLengthInNBO), 0);
	// printf("send byte: %d\n",sendByte);
	
	sendString(fd, username, usernameLength);
}

void sendUpdateToAllUsers(struct username * users, uint16_t numberOfUsers, char* name, uint8_t nameLen,  uint8_t flag)
{
	printf("in send leave update\n");

	uint8_t usernameLength = nameLen;
	char username[usernameLength];
	strcpy(username, name);

	int index;
	for(index = 0; index < numberOfUsers; index++) 
	{
		int fd = users[index].fd;

		if (flag == ((uint8_t)0x02))
		{
			// int flag = htonl(0x02); // send leave message flag
			printf("sending leave flag: %x\n", flag);
		}
		else if (flag == ((uint8_t)0x01)){
			// int flag = htonl(0x01);
			printf("sending join flag: %x\n", flag);
		}
		
		send(fd, &flag, sizeof(flag), 0);
		printf("sent flag\n");

		sendUsernameAndLength(fd, users, numberOfUsers, username, usernameLength);

		// send(fd, message, sizeof(message), 0);
		printf("sent user update: %s to fd: %d \n",username, fd);
	}
}

void receiveString(int s, char * str, int len)
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
	printf("Received String: %s\n", str);
}

int main(void)
{
	// int newfd; // max file descriptor number, listening socket descriptor, newly accepted sockect descriptor
		
	// char buf[256]; // data for client buffer
	// int nbytes;

	// int i, j,rv;
	struct username user; // used to get size
	struct username * users = malloc(1 * sizeof(user));
	uint16_t numberOfUsers = 0;

	int listener = getListener();

	// get us a socket and bind it
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(MY_PORT);

	int yes = 1; // set the socket to re-use the address
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
		perror("Server: cannot set socket option");
		exit(1);
	}

	if (bind(listener, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
		perror ("Server: cannot bind master socket");
		exit(1);
	}

	if(listen (listener, 10) == -1)
	{
		perror("Server: cannot listen");
		exit(1);
	}

	fd_set master, read_fds; // master file descriptor list, and temp file descriptor list
	FD_ZERO(&master); // clear the master and temp set
	FD_ZERO(&read_fds);

	FD_SET(listener, &master); // add the listener to the master set
	int fdmax = listener; // keep track of the biggest file descriptors, for now its the listener

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	while (1) // -------------------------------------------------------- while loop --------------------------------------------------
	{
		//fprintf(stderr, "========= Starting while loop =========\n");

		read_fds = master; // copy it

		if(select(fdmax+1, &read_fds, NULL, NULL, &timeout) == -1)
		{
			perror("Server: cannot select file descriptor");
			exit(1);
		}

		int i;
		for(i = 0; i <= fdmax; i++) // this loops through the file descriptors
		{
			if(FD_ISSET(i, &read_fds)) // we got one!!
			{
				if(i == listener) // handle new connection!
				{
					struct sockaddr_in remoteaddr; // the remoteaddr is the client addr
					socklen_t addrlen = sizeof(remoteaddr);

					int newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);

					if (newfd < 0) 
					{
						perror ("Server: accept failed");
						continue;
					}
					
					sendInitialHandshake(newfd);
					sendNumberOfUsers(newfd, numberOfUsers);
					sendAllUserNames(newfd, users, numberOfUsers);

					int usernameLen = getUsernameLength(newfd);
					char username[usernameLen + 1]; // required for adding a null terminator
					recv(newfd, &username, usernameLen, 0);
					username[usernameLen] = '\0';
					printf("Client username: %s\n", username);

					if(isUniqueUsername(users, numberOfUsers, username))
					{
						numberOfUsers = numberOfUsers + 1;
						users = realloc(users, numberOfUsers * sizeof(user));

						printf("Adding user: %s\n", username);
						addUserName(users, numberOfUsers, username, usernameLen, newfd); //Calvin: changed i tp newfd
						sendUpdateToAllUsers(users, numberOfUsers, username, usernameLen, 1);

						printUsers(users, numberOfUsers);
						printf("\n");
					}
					else
					{
						fprintf(stderr, "\n...Username is not unique, closing connection\n");
						close(newfd);
						continue; // skip adding it to the master set
					}

					// successfully accepted a new selection!
					FD_SET(newfd, &master); // add to the master set
					if(newfd > fdmax)
						fdmax = newfd; // leep track of the max

					printf("Selected Server: new connection from %s:%d on socket %d\n", inet_ntoa(remoteaddr.sin_addr), ntohs(remoteaddr.sin_port), newfd); // cant print off somethings...
				}
				else 
				{ // handling data from clients!!
					char buf[256]; // data for client buffer, their messages
					int nbytes;
					uint16_t messageLength;

					if((nbytes = recv(i, &messageLength, sizeof(messageLength), 0)) <= 0)
					{
						if(nbytes == 0) // got error or connection closed by client
						{
							printf("Selected Server: socket %d hung up\n", i);
						}
						else
						{
							printf("Error: could not recv from client\n");
						}

						int index = getUserIndex(users, numberOfUsers, i);

						uint8_t usernameLen = users[index].length;
						char username[usernameLen];
						strcpy(username, users[index].name);

						users = deleteUser(users, numberOfUsers, i); //delete user that disconnected
						numberOfUsers = numberOfUsers -1;
						sendUpdateToAllUsers(users, numberOfUsers, username, usernameLen, 2);

						printf("after deleteUser\n");
						printUsers(users, numberOfUsers);
						close(i);
						FD_CLR(i, &master);
					}
					else
					{
						// get message and send to everyone...
						messageLength = ntohs(messageLength);
						printf("message length: %d\n", messageLength);

						char message[messageLength + 1]; // required for adding a null terminator
						receiveString(i, message, messageLength);
						printf("message recieved: %s\n", message);

						int index;
						for(index = 0; index < numberOfUsers; index++)
						{
							if(users[index].fd == i) // found user!
							{
								break;
							}
						}

						sendMessageToAllUsers(users, numberOfUsers, users[index].name, users[index].length, messageLength, message);

						// int index;
						// for(index = 0; index < numberOfUsers; index++) 
						// {
						// 	int fd = users[index].fd;

						// 	int messageLengthInNBO = htons(messageLength);
						// 	int sendByte = send(fd, &messageLengthInNBO, sizeof(messageLengthInNBO), 0);
						// 	printf("send byte: %d\n",sendByte);
						// 	int k;
						// 	for(k = 0; k < messageLength; k++)
						// 	{
						// 		int byteSent = send(fd, &message[k], sizeof(message[k]), 0);
						// 		printf("%d byte sent, character sent: %c\n", byteSent, message[k]);
						// 	}

						// 	// send(fd, message, sizeof(message), 0);
						// 	printf("sent message: %s to fd: %d \n",message, fd);
						// }
					}
					// int keepAliveTime = 3000;
					// clock_t before = clock()*1000/CLOCKS_PER_SEC;
					// while(1)
					// {
					// 	int keepAlive;
					// 	int current = clock()*1000/CLOCKS_PER_SEC;
					// 	if((current - before) >= keepAliveTime)
					// 	{
					// 		if(recv(newfd, &keepAlive, sizeof(keepAlive), 0) == 0 ||
					// 			recv(newfd, &keepAlive, sizeof(keepAlive), 0) == -1)
					// 		{
					// 			printf("Closing socket connection with client\n");
					// 			close(newfd);
					// 			break;
					// 		}

					// 		if(ntohl(keepAlive) == 0)
					// 			printf("recieved keep alice\n");
					// 		before = clock()*1000/CLOCKS_PER_SEC;
					// 	}
					// }
				} // ending if/else to handle data from the client
			} // ending if got a new incoming connection
		} // ending for loop, going through all file descriptors
	}  // ending while loop

	return 0;
}
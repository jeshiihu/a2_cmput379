
#include "server.h"

/* ---------------------------------------------------------------------
 This	is  a sample server which opens a stream socket and then awaits
 requests coming from client processes. In response for a request, the
 server sends an integer number  such	 that  different  processes  get
 distinct numbers. The server and the clients may run on different ma-
 chines.
 --------------------------------------------------------------------- */
void printUsers(struct username * users, int numberOfUsers)
{
	printf("Number of users: %d\n", numberOfUsers);
	
	int i;
	for(i = 0; i < numberOfUsers; i++)
		printf("users[%d] = %s\n", i, users[i].name);
}

void addUserName(struct username * users, int size, char* name, int nameLen, int fd)
{
	users[size - 1].length = nameLen;
	users[size - 1].fd = fd;

	users[size - 1].name = malloc(nameLen * sizeof(char));
	strcpy(users[size - 1].name, name);
}

void deleteUser(struct username * users, int size, int fd)
{
	// int i;
	// struct username user;

	// for(i = 0; i < size; i++)
	// {
	// 	if(users[i].fd == fd) // found the user
	// 	{

	// 	}
	// 	else
	// 	{
	// 		temp[i]
	// 	}
	// }
}

void sendInitialHandshake(int listener)
{
	printf("sending handshake...\n" );

	int outnum = htonl(0xCF);
	send (listener, &outnum, sizeof (outnum), 0);
	
	outnum = htonl (0xA7);
	send (listener, &outnum, sizeof (outnum), 0);
}

void sendNumberOfUsers(int listener, int numUsers)
{
	printf("sending number of users...\n" );

	int num = htons(numUsers);
	send (listener, &num, sizeof(num), 0);
}

int getUsernameLength(int listener)
{
	int i;
	recv(listener, &i, sizeof(i), 0);

	return i;
}

bool isUniqueUsername(struct username * users, int size, char* newUser)
{
	int i;
	for (i = 0; i < size; i++) 
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

int main(void)
{
	// int newfd; // max file descriptor number, listening socket descriptor, newly accepted sockect descriptor
		
	// char buf[256]; // data for client buffer
	// int nbytes;

	// int i, j,rv;
	struct username user; // used to get size
	struct username * users = malloc(1 * sizeof(user));
	int numberOfUsers = 0;

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

	while (1) // -------------------------------------------------------- while loop --------------------------------------------------
	{
		fprintf(stderr, "========= Starting while loop =========\n");

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

					int usernameLen = getUsernameLength(newfd);
					char username[usernameLen + 1]; // required for adding a null terminator
					recv(newfd, &username, usernameLen, 0);
					username[usernameLen] = '\0';
					printf("Client username: %s\n",username);

					if(isUniqueUsername(users, numberOfUsers, username))
					{
						numberOfUsers = numberOfUsers + 1;
						users = realloc(users, numberOfUsers * sizeof(user));

						printf("Adding user: %s\n", username);
						addUserName(users, numberOfUsers, username, usernameLen, i);

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
					if(nbytes = recv(i, buf, sizeof(buf), 0) <= 0)
					{
						if(nbytes == 0) // got error or connection closed by client
						{
							printf("Selected Server: socket %d hung up\n", i);
						}
						else
						{
							printf("Error: could not recv from client\n");
						}

						deleteUser(users, numberOfUsers, i);

						close(i);
						FD_CLR(i, &master);
					}
					else
					{
						// get message and send to everyone...
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
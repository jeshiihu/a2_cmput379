
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
	fprintf(stderr, "Number of users: %d\n", numberOfUsers);
	
	int i;
	for(i = 0; i < numberOfUsers; i++)
		fprintf(stderr, "users[%d] = %s\n", i, users[i].name);
}

void addUserName(struct username * users, int size, char* name, int nameLen)
{
	users[size - 1].length = nameLen;
	users[size - 1].name = malloc(nameLen * sizeof(char));
	strcpy(users[size - 1].name, name);
}

void sendInitialHandshake(int sock)
{
	fprintf(stderr, "sending handshake...\n" );

	int outnum = htonl(0xCF);
	send (sock, &outnum, sizeof (outnum), 0);
	
	outnum = htonl (0xA7);
	send (sock, &outnum, sizeof (outnum), 0);
}

void sendNumberOfUsers(int sock, int numUsers)
{
	fprintf(stderr, "sending number of users...\n" );

	int num = htons(numUsers);
	send (sock, &num, sizeof(num), 0);
}

int getUsernameLength(int sock)
{
	int i;
	recv(sock, &i, sizeof(i), 0);

	return i;
}

bool isUniqueUsername(struct username * users, int size, char* newUser)
{
	int i;
	for (i = 0; i < size; i++) 
	{
		fprintf(stderr, "comparing: %s and %s \n", users[i].name, newUser);
		if (strcmp(users[i].name, newUser) == 0) // two names match so invalid username since not unqiue
			return false;
	}

	return true;
}

int main(void)
{
	struct username user; // used to get size
	struct username * users = malloc(1 * sizeof(user));
	int numberOfUsers = 0;

	int sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}

	struct sockaddr_in master;
	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (MY_PORT);

	int yes = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
		perror("cant set socket option");
		exit(1);
	}

	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		perror ("Server: cannot bind master socket");
		exit (1);
	}

	listen (sock, 5);

	while (1) {
		fprintf(stderr, "Starting while loop\n");

		struct sockaddr_in from;
		int fromlength = sizeof (from);
		int snew = accept (sock, (struct sockaddr*)&from, (socklen_t *)&fromlength);

		if (snew < 0) {
			perror ("Server: accept failed");
			exit (1);
		}

		sendInitialHandshake(snew);
		sendNumberOfUsers(snew, numberOfUsers);

		int usernameLen = getUsernameLength(snew);
		char username[usernameLen + 1]; // required for adding a null terminator
		recv(snew, &username, usernameLen, 0);
		username[usernameLen] = '\0';
		fprintf(stderr, "Client username: %s\n",username);

		if(isUniqueUsername(users, numberOfUsers, username))
		{
			numberOfUsers = numberOfUsers + 1;
			users = realloc(users, numberOfUsers * sizeof(user));
			addUserName(users, numberOfUsers, username, usernameLen);
			printUsers(users, numberOfUsers);
		}
		else
		{
			fprintf(stderr, "\n...Username is not unique, closing connection\n");
			close(snew);
		}

		// int keepAliveTime = 3000;
		// clock_t before = clock()*1000/CLOCKS_PER_SEC;
		// while(1)
		// {
		// 	int keepAlive;
		// 	int current = clock()*1000/CLOCKS_PER_SEC;
		// 	if((current - before) >= keepAliveTime)
		// 	{
		// 		if(recv(snew, &keepAlive, sizeof(keepAlive), 0) == 0 ||
		// 			recv(snew, &keepAlive, sizeof(keepAlive), 0) == -1)
		// 		{
		// 			fprintf(stderr, "Closing socket connection with client\n");
		// 			close(snew);
		// 			break;
		// 		}

		// 		if(ntohl(keepAlive) == 0)
		// 			fprintf(stderr, "recieved keep alice\n");
		// 		before = clock()*1000/CLOCKS_PER_SEC;
		// 	}
		// }

			fprintf(stderr, "outside while loop, &users = %p\n", (void*)&users);
			printUsers(users, numberOfUsers);

			close(snew);
			fprintf(stderr, "\n");
	}
}
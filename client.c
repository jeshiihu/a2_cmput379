
#include "client.h"
/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.				 
 --------------------------------------------------------------------- */

int main(int argc, char** argv)
{
	int	s, number;

	struct	sockaddr_in	server;

	struct	hostent	*host;

	host = gethostbyname("129.128.41.52");

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
		server.sin_port = htons (MY_PORT);

		if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
			perror ("Client: cannot connect to server");
			exit (1);
		}

		// if flag is 0 then it works exactly like read();
		int firstByte, secondByte;
		recv(s, &firstByte, sizeof (firstByte),0);
		fprintf (stderr, "Process: %d, first byte: %x\n", getpid (), ntohl (firstByte));
		
		recv(s, &secondByte, sizeof(secondByte),0);
		fprintf(stderr, "Process: %d, gets number: %x\n", getpid (), ntohl(secondByte));

		if(ntohl(firstByte) == 0xcf && ntohl(secondByte) == 0xa7)
		{
			fprintf(stderr, "confirm\n");
			int numberOfUsers;
			recv(s, &numberOfUsers, sizeof(secondByte),0);
			fprintf(stderr, "Number of Users:  %d\n", ntohs(numberOfUsers));
			
			int i;
			for(i = 1; i < argc; i++)
			{
				int usernameLen = (int)strlen(argv[i]);
				send(s, &usernameLen, sizeof(usernameLen), 0); // send username length
				send(s, argv[i], sizeof(argv[i]), 0);
				fprintf(stderr, "Name: %s\n", argv[i]);
			}

			// int keepAliveTime = 3000;
			// clock_t before = clock()*1000/CLOCKS_PER_SEC;
			while(1)
			{
				// int current = clock()*1000/CLOCKS_PER_SEC;
				// if((current - before) >= keepAliveTime)
				// {
				// 	fprintf(stderr, "Clock: %d, Before: %d\n", current, before);
				// 	int keepAliveLen = htonl (0);
				// 	if(send (s, &keepAliveLen, sizeof(keepAliveLen), 0) == -1)
				// 	{	
				// 		fprintf(stderr, "Connection closed.\n");
				// 		close(s);
				// 	}

				// 	before = clock()*1000/CLOCKS_PER_SEC;
				// }	
			}
		}

		// keep alive
		sleep(5);
		close(s);
	}
}

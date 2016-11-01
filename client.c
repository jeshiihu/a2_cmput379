
#include "client.h"
/* ---------------------------------------------------------------------
 This is a sample client program for the number server. The client and
 the server need not run on the same machine.				 
 --------------------------------------------------------------------- */
char *inputMessage(FILE* fp, size_t size){
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


int main(int argc, char** argv)
{
	if(argc != 5)
	{
		printf("Invalid client input. Should be formatted as: chatname hostname portnumber username\n");
		return 0; 
	}

	// get the client inputs!
	char chatname[strlen(argv[1])];
	strcpy(chatname, argv[1]);

	char hostname[strlen(argv[2])];
	strcpy(hostname, argv[2]);
	
	char portnumber[strlen(argv[3])];
	strcpy(portnumber, argv[3]);
	
	char username[strlen(argv[4])];
	strcpy(username, argv[4]);

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
			
			int len = (int)strlen(username);
			send(s, &len, sizeof(len), 0); // send username length
			send(s, username, sizeof(username), 0);
			fprintf(stderr, "Name: %s\n", username);

			// int keepAliveTime = 3000;
			// clock_t before = clock()*1000/CLOCKS_PER_SEC;
				pid_t forkID = fork();
			while(1)
			{
				// printf("while\n");
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
					}
				}
				else // child process wants to always listen to the incoming messages
				{
				// printf("message sent: %s\n", message);
				// printf("message length: %d\n", messageLength);
					free(message);

					uint16_t recvMessageLength;
					int recvByte;
					if (recvByte = (recv(s, &recvMessageLength, sizeof(recvMessageLength), MSG_DONTWAIT) > 0)){
						recvMessageLength = ntohs(recvMessageLength);
						// printf("message recieved length: %d\n",recvMessageLength );
						char recvMessage[recvMessageLength + 1];
						
						int currentBytesRead = 0;
						while(currentBytesRead < recvMessageLength)
						{
							char c;
							int bytesReceived = recv(s, &c, sizeof(c), 0);

							if (strlen(&c) != 0) {
								recvMessage[currentBytesRead] = c;
								currentBytesRead = currentBytesRead + bytesReceived;
							}
						}

						recvMessage[recvMessageLength] = '\0';
						printf("message recieved: %s\n", recvMessage);
					}
				}
				//char inputMessage[250];
				//scanf("%s", inputMessage);
				//printf("after scanf\n");
				//char message[strlen(inputMessage)];
				//printf("message length: %d\n", strlen(inputMessage));
				//strcpy(message, inputMessage);
				//printf("after copy\n");
				//printf("%s\n,", message);

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

	return 0;
}

HEADERS = server.h

CC = gcc

SOURCES = server.c client.c

all: server client

server: server.c
	gcc -o server server.c

client: client.c
	gcc -o client client.c

clean:
	-rm -f *.o server client

run:
	./server
	./client
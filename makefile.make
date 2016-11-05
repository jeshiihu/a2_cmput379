HEADERS = server.h client.h

CC = gcc

SOURCES = server.c client.c

all: server client

server: server.c
	gcc -o server379 server.c

client: client.c
	gcc -o chat379 -pthread client.c

clean:
	-rm -f *.o server client

run:
	./server379
	./chat379

CC=gcc
CFLAGS=-W -Wall -ansi -g
LDFLAGS=-lreadline -lpthread


all: server client


server: src/server.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

client: src/client.c
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm -f server client

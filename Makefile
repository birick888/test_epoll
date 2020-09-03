CC = gcc
CFLAGS  = -g -Wall -std=gnu99
LIBS = -lpthread

#client: client.c
#	$(CC) $(CFLAGS) -o client client.c
#server: server.c
#	$(CC) $(CFLAGS) -o server server.c $(LIBS)

all: server.c client.c 
	$(CC) $(CFLAGS) -o server server.c $(LIBS)
	$(CC) $(CFLAGS) -o client client.c $(LIBS)
clean:
	-rm -f server client *.o core *.core

CC = gcc
CFLAGS  = -g -Wall
LIBS = -lpthread

#client: client.c
#	$(CC) $(CFLAGS) -o client client.c
#server: server.c
#	$(CC) $(CFLAGS) -o server server.c $(LIBS)

all: server.c client.c 
	gcc -g -Wall -o server server.c $(LIBS)
	gcc -g -Wall -o client client.c
clean:
	-rm -f server client *.o core *.core

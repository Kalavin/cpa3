all: client server

client: client.c client.h
	gcc -Wall -g -o client client.c -lpthread

server: server.c server.h
	gcc -Wall -g -o server server.c -lpthread

clean:
	rm -rf server client *.o

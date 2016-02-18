all: server client

server: server.o
	gcc -g server.o -o server -lpthread

server.o: server.c
	gcc -g -c server.c

client: client.o
	gcc -g client.o -o client

client.o: client.c
	gcc -g -c client.c


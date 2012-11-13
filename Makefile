default: all

all: client server

client: client.c
	gcc client.c -o client.exe

server: server.c
	gcc -lpthread server.c -o server.exe
CC = gcc
CFLAGS = -Wall -std=gnu99 -lpthread

all: client server

client: client.c
server: server.c
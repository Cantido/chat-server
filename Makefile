CFLAGS = -Wall -std=gnu99
LDLIBS = -lpthread

all: client server

client: client.c
server: server.c

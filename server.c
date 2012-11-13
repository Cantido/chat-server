#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

/* socket()
 * bind()
 * listen()
 * accept()
 * -> clients connect() now
 * read & write
 * close()
 */

#define SERVER_PORT 9999
#define SERVER_HOST "localhost"
#define MAX_CLIENTS 10

void *client_thread(void *arg);

int main() {
	int stream_socket;
	pthread_t clients[MAX_CLIENTS];
	
	struct sockaddr_in server_address = { AF_INET, htons(SERVER_PORT) };
	struct sockaddr_in client_address = { AF_INET };
	
	int server_length = sizeof(server_address);
	int client_length = sizeof(client_address);
	
	stream_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	bind(stream_socket, (struct sockaddr *) &server_address, server_length);
	
	listen(stream_socket, MAX_CLIENTS);
	
	printf("Server is now listening for connections.\n");
	
	clients[0] = client_thread ((void *) accept(stream_socket, (struct sockaddr*) &client_address, &client_length));
	
	pthread_join(clients[0], NULL);
	
	close(stream_socket);
	
	return(0);
}

// params: int from accept()

void *client_thread (void *arg) {
	
	printf("Server has accepted a connection.\n");
	
	char buf[512];
	int chars_read;
	
	int socket = (int) arg;
	
	while((chars_read = read(socket, buf, sizeof(buf))) > 0) {
		printf("Server recieved: %s\n", buf);
		write(socket, buf, chars_read);
	}
}
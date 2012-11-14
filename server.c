#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

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
	
	int num_clients = 0;
	int server_running = 1;
	
	stream_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	
	bind(stream_socket, (struct sockaddr *) &server_address, server_length);
	
	listen(stream_socket, MAX_CLIENTS);
	
	printf("Server is now listening for connections.\n");
	
	
	while ((server_running == 1) && (num_clients <= 10)) {
		int client_socket = accept(stream_socket, (struct sockaddr*) &client_address, &client_length);
		
		if (client_socket > 0) {
			pthread_create(&clients[num_clients++], NULL, client_thread, (void *) client_socket);
		}
	}
	
	close(stream_socket);
	
	return(0);
}

// params: int from accept()

void *client_thread (void *arg) {
	
	printf("Server has accepted a connection.\n");
	
	char buf[512];
	int chars_read;
	
	int socket = (int) arg;
	
	fcntl(socket, F_SETFL, ~O_NONBLOCK);
	
	while((chars_read = read(socket, buf, sizeof(buf))) > 0) {
		printf("Server recieved: %s\n", buf);
		write(socket, buf, chars_read);
	}
	
	printf("Thread exiting.\n");
}

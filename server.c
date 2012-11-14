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

#define DEBUG 1

#ifdef DEBUG
	#define DPRINT(x) printf(x);
#else
	#define DPRINT(x)
#endif
 
#define SERVER_PORT 9999
#define SERVER_HOST "localhost"
#define MAX_CLIENTS 10

void *client_thread(void *arg);
void handler(int signum);
int server_running = 1;

struct client {
	pthread_t tid;
	int socket;
	int alive; // 1 if the thread is still running, else 0
};

int main() {
	int stream_socket;
	
	struct sockaddr_in server_address = { AF_INET, htons(SERVER_PORT) };
	struct sockaddr_in client_address = { AF_INET };
	
	int server_length = sizeof(server_address);
	int client_length = sizeof(client_address);
	
	int num_clients = 0;
	struct client *clients[MAX_CLIENTS] = { NULL };
	
	signal(SIGINT, handler);
	
	/* establish the socket */
	
	stream_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	
	bind(stream_socket, (struct sockaddr *) &server_address, server_length);
	
	listen(stream_socket, MAX_CLIENTS);
	
	/* MAIN LOOP : Accept connections, spawn threads for connections, etc */
	
	printf("Server is now listening for connections.\n");
	
	
	while ((server_running == 1) && (num_clients <= 10)) {
		/* remember that stream_socket has been made non-blocking */
		int client_socket = accept(stream_socket, (struct sockaddr*) &client_address, &client_length);
		
		/* if we accepted a connection, spawn the thread */
		if (client_socket > 0) {
			for(int i = 0; i < MAX_CLIENTS; i++) {
				if(clients[i] == NULL) {
					clients[i] = (struct client *) malloc(sizeof(struct client));
					clients[i]->alive = 1;
					clients[i]->socket = client_socket;
			
					pthread_create(&(clients[i]->tid), NULL, client_thread, (void *) clients[i]);
				
					break;
				}
			}
		}
		
		/* END accepted new connection */
		
		/* Scan for clients that have disconnected (set alive = 0); */
		
		for(int i = 0; i < MAX_CLIENTS; i++) {
			if((clients[i] != NULL) && (clients[i]->alive == 0)) {
				printf("Client %d has disconnected.\n", i);
				pthread_join(clients[i]->tid, NULL);
				free(clients[i]);
				clients[i] = NULL;
			}
		}
	}
	
	printf("Server shutting down.\n");
	
	close(stream_socket);
	
	return(0);
}

void *client_thread (void *arg) {
	
	printf("Server has accepted a connection.\n");
	
	char buf[512];
	int chars_read;
	
	int socket = ((struct client *) arg)->socket;
	
	fcntl(socket, F_SETFL, ~O_NONBLOCK);
	
	while((chars_read = read(socket, buf, sizeof(buf))) > 0) {
		printf("Server recieved: %s\n", buf);
		write(socket, buf, chars_read);
	}
	
	printf("Thread exiting.\n");
	((struct client *) arg)->alive = 0;
	
	return NULL;
}

void handler(int signum) {
	if(signum == SIGINT) {
		printf("Caught SIGINT.\n");
		server_running = 0;
	}	
}

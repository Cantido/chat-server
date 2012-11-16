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
#include <semaphore.h>
#include <stdarg.h>

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
void handler(int signum);
void print_to_all (const char* format, ...);
void print_to_clients (const char* format, ...);

struct client {
	pthread_t tid;
	int socket;
};

struct client *clients[MAX_CLIENTS] = { NULL };

sem_t client_array_sem;

int main() {
	int stream_socket;
	int client_socket;
	
	struct sockaddr_in server_address = { AF_INET, htons(SERVER_PORT) };
	struct sockaddr_in client_address = { AF_INET };
	
	int server_length = sizeof(server_address);
	int client_length = sizeof(client_address);
	
	signal(SIGINT, handler);
	
	sem_init(&client_array_sem, 0, 1);
	
	/* establish the socket */
	
	stream_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	bind(stream_socket, (struct sockaddr *) &server_address, server_length);
	
	listen(stream_socket, MAX_CLIENTS);
	
	/* set up a pipe to allow all threads to communicate back to the main server */
	
	/* MAIN LOOP : Accept connections, spawn threads for connections, etc */
	
	printf("Server is now listening for connections.\n");
	
	
	while ((client_socket = accept(stream_socket, (struct sockaddr*) &client_address, &client_length)) > 0) {
			
		sem_wait(&client_array_sem);

		for(int i = 0; i < MAX_CLIENTS; i++) {
			
			if(clients[i] == NULL) {
				
				clients[i] = (struct client *) malloc(sizeof(struct client));
				clients[i]->socket = client_socket;
		
				pthread_create(&(clients[i]->tid), NULL, client_thread, (void *) &i);
				
				sem_post(&client_array_sem);
				break;
			}
		}
		/* END accepted new connection */
	}
	
	printf("Server shutting down.\n");
	
	close(stream_socket);
	
	return(0);
}

void *client_thread (void *arg) {
	
	char buf[BUFSIZ];
	int chars_read;
	
	int index = *((int *) arg);
	
	
	sem_wait(&client_array_sem);
	int thread_socket = clients[index]->socket;
	sem_post(&client_array_sem);
	
	print_to_all("User %d has connected.\n", index);
	
	/* The strings sent by the client are already terminated with newlines */
	
	while((chars_read = read(thread_socket, buf, BUFSIZ)) > 0) {
		char user_says[BUFSIZ];
		
		sprintf(user_says, "User %d: %s", index, buf);
		
		print_to_all(user_says);
	}
	
	print_to_all("User %d disconnected.\n", index);
	
	sem_wait(&client_array_sem);
	free(clients[index]);
	clients[index] = NULL;
	sem_post(&client_array_sem);
	
	return NULL;
}

void handler(int signum) {
	time_t time_to_wait = 0;
	
	if(signum == SIGINT) {
		printf("Caught SIGINT. Shutting down server.\n");
	}
	
	print_to_all ("Server will shut down in %d seconds.\n", time_to_wait);
	
	sleep(time_to_wait);
	
	sem_wait(&client_array_sem);
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] != NULL) {
			pthread_cancel(clients[i]->tid);
			pthread_join(clients[i]->tid, NULL);
			free(clients[i]);
			clients[i] = NULL;
		}
	}
	sem_post(&client_array_sem);
	
	exit(0);
}

void print_to_all (const char* format, ...) {
	char buf[BUFSIZ];
	
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	
	printf(buf); // this is the only difference between this and print_to_clients
	
	sem_wait(&client_array_sem);
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] != NULL) {
			write(clients[i]->socket, buf, BUFSIZ);
		}
	}
	sem_post(&client_array_sem);
}

void print_to_clients (const char* format, ...) {
	char buf[BUFSIZ];
	
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	
	sem_wait(&client_array_sem);
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] != NULL) {
			write(clients[i]->socket, buf, BUFSIZ);
		}
	}
	sem_post(&client_array_sem);
}

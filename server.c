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
 
#define SERVER_PORT 9999
#define SERVER_HOST "localhost"
#define MAX_CLIENTS 10
#define SHUTDOWN_WAIT 0

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

/* client_thread
 *
 * Desc: reads to & writes from clients. One thread reads from one client but prints to all clients.
 *
 * Parameters: arg: a pointer to a struct client
 *
 * Returns: nothing
 *
 */
 
void *client_thread (void *arg) {
	
	char buf[BUFSIZ];
	char user_name[BUFSIZ];
	int chars_read;
	
	int index = *((int *) arg);
	
	
	sem_wait(&client_array_sem);
	int thread_socket = clients[index]->socket;
	sem_post(&client_array_sem);
	
	read(thread_socket, user_name, BUFSIZ);
	
	print_to_all("* %s has connected. *\n", user_name);
	
	while((chars_read = read(thread_socket, buf, BUFSIZ)) > 0) {
		char user_says[BUFSIZ];
		
		sprintf(user_says, "%s: %s\n", user_name, buf);
		
		print_to_all(user_says);
	}
	
	print_to_all("* %s has disconnected. *\n", user_name, index);
	
	sem_wait(&client_array_sem);
	free(clients[index]);
	clients[index] = NULL;
	sem_post(&client_array_sem);
	
	return NULL;
}

/* handler
 *
 * Desc: signal handler
 *
 * Parameters: signal: the number of the signal to handle (SIGINT == 11)
 *
 * Side-effects: Shuts down the server after a time defined by SHUTDOWN_WAIT
 *
 * Returns: nothing
 *
 */

void handler(int signum) {
	
	if(signum == SIGINT) {
		printf("Caught SIGINT. Shutting down server.\n");
	}
	
	print_to_all ("Server will shut down in %d seconds.\n", SHUTDOWN_WAIT);
	
	sleep(SHUTDOWN_WAIT);
	
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


/* print_to_all
 *
 * Desc: Prints a formatted string to all connected clients and to the server stdout.
 *       Behaves exactly like print_to_clients except this also prints to the server stdout.
 *		 This and print_to_clients behave exactly like printf, except they print to connected clients.
 *
 * Parameters: format: formatted string like in printf
 *             ...   : token values, like in printf
 *
 * Returns: nothing
 *
 */

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

/* print_to_clients
 *
 * Desc: Prints a formatted string to all connected clients..
 *       Behaves exactly like print_to_all except this this doesn't print to to the server stdout.
 *		 This and print_to_all behave exactly like printf, except they print to connected clients.
 *
 * Parameters: format: formatted string like in printf
 *             ...   : token values, like in printf
 *
 * Returns: nothing
 *
 */

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

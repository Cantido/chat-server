#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 9999

void *read_from_server(void *arg);
void handler(int signal);
 
int main() {
	/* socket variables */
	int stream_socket;
	struct sockaddr_in server_address = { AF_INET, htons(SERVER_PORT) };
	int server_length = sizeof(server_address);
	struct hostent *hp;
	
	/* other technical variables */
	pthread_t tid;
	char buf[BUFSIZ];
	
	/* variables for pretty menus and error checking */
	unsigned char verify = 'n';
	int success_flag = 0;
	
	/* END variable declarations */
	
	
	/* Set up signal handler */
	signal(SIGINT, handler);
	/* END set up signal handler */
	
	
	/* Query user for server address, then check & connect to address */
	
	do {
		verify = 'n';
		
		printf("Server Address: ");
		fgets(buf, BUFSIZ, stdin);
		buf[strlen(buf)-1] = '\0'; // removing the newline that fgets has read in
		printf("Is this the correct host name: \"%s\"? (Y/n) > ", buf);
		verify = (unsigned char) getchar();
		
		if (verify == 'y' || verify == 'Y') {
			success_flag = 1;
		
			hp = gethostbyname(buf);
	
			if((memcpy((char*) &server_address.sin_addr, hp->h_addr_list[0], hp->h_length)) == NULL) {
				perror("Error (location 1)");
				success_flag = 0;
			}
	
			if ((stream_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				perror("Error (location 2)");
				success_flag = 0;
			}
	
			if((connect(stream_socket, (struct sockaddr*) &server_address, server_length)) == -1) {
				perror("Error (location 3)");
				success_flag = 0;
			}
		}
		getchar(); // there's an extra newline hanging around here for some reason. this clears it
	} while (success_flag != 1);
	
	memset(buf, 0, BUFSIZ);
	
	/* END connection routine */
	
	
	/* Query user for username */
	
	do {
		printf("Desired Username: ");
		fgets(buf, BUFSIZ, stdin);
		buf[strlen(buf)-1] = '\0'; // removing the newline that fgets has read in
		printf("Is this the correct username: \"%s\"? (Y/n) > ", buf);
		verify = (unsigned char) getchar();
	} while (verify != 'y' && verify != 'Y');
	
	write(stream_socket, buf, BUFSIZ); // sending username to server
	
	getchar(); // there's an extra newline hanging around here for some reason. this clears it
	
	/* END username read */
	
	
	/* MAIN LOOPS: read_from_server thread and read from console loop */
	
	pthread_create(&tid, NULL, read_from_server, (void *) &stream_socket);
	
	while(fgets(buf, BUFSIZ, stdin) != NULL) {
		buf[strlen(buf)-1] = '\0'; // removing the newline that fgets has read in
		if (
			(strcmp(buf, "/exit") == 0) ||
			(strcmp(buf, "/quit") == 0) || 
			(strcmp(buf, "/part") == 0)
		   )
			break;
		
		write(stream_socket, buf, BUFSIZ);
	}
	
	/* END main loops */
	
	
	/* tear down */
	
	pthread_cancel((pthread_t) tid);
	pthread_join(tid, NULL);
	
	printf("Disconnected from server. Exiting program.\n");
	
	close(stream_socket);
	return(0);
}

/* read_from_server
 *
 * Desc: Thread to read from the server socket and print it.
 *
 * Parameters: arg: a pointer to the integer file descriptor of the socket from which to read.
 *
 * Returns: nothing
 *
 */

void *read_from_server(void *arg) {
	char buf[BUFSIZ];
	int chars_read;
	int stream_socket = *((int *) arg);
	while((chars_read = read(stream_socket, buf, BUFSIZ)) > 0) {
		printf(buf);
	}
	
	printf("The server has disconnected.\n");

	exit(EXIT_SUCCESS);
}

/* handler
 *
 * Desc: signal handler
 *
 * Parameters: signal: the number of the signal to handle (SIGINT == 11)
 *
 * Returns: nothing
 *
 */
 
void handler(int signal) {
	if (signal == SIGINT) {
		printf("*** Please use /exit, /quit, or /part to close the program ***\n");
	}
}
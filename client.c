#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

/* socket()
 * connect()
 * read & write
 * close()
 */

#define SERVER_PORT 9999
#define HOST_NAME "localhost"

void *read_from_server(void *arg);
 
int main() {
	int streamSocket;
	int tid;
	
	struct sockaddr_in serverAddress = { AF_INET, htons(SERVER_PORT) };
	
	int serverLength = sizeof(serverAddress);
	
	struct hostent *hp;

	char buf[BUFSIZ];
	int charsRead;
	
	hp = gethostbyname(HOST_NAME);
	
	memcpy((char*) &serverAddress.sin_addr, hp->h_addr_list[0], hp->h_length);
	
	streamSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	if(connect(streamSocket, (struct sockaddr*) &serverAddress, serverLength) == -1) {
		perror("Client error:");
		exit(1);
	}
	
	printf("Client has connected to the server.\n");
	
	pthread_create(&tid, NULL, read_from_server, (void *) &streamSocket);
	
	while(fgets(buf, BUFSIZ, stdin) != NULL) {
		write(streamSocket, buf, BUFSIZ);
	}
	
	printf("Exiting.\n");
	
	close(streamSocket);
	return(0);
}

void *read_from_server(void *arg) {
	char buf[BUFSIZ];
	int chars_read;
	int stream_socket = *((int *) arg);
	while((chars_read = read(stream_socket, buf, BUFSIZ)) > 0) {
		printf(buf);
	}
	
	printf("Disconnected from the server.\n");
	return NULL;
}
		

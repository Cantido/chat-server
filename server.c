#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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

int main() {
	int streamSocket;
	int clientSocket;
	
	struct sockaddr_in serverAddress = { AF_INET, htons(SERVER_PORT) };
	struct sockaddr_in clientAddress = { AF_INET };
	
	int serverLength = sizeof(serverAddress);
	int clientLength = sizeof(clientAddress);
	
	char buf[512];
	int charsRead;
	
	streamSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	bind(streamSocket, (struct sockaddr *) &serverAddress, serverLength);
	
	listen(streamSocket, 1);
	
	printf("Server is now listening for connections.\n");
	
	clientSocket = accept(streamSocket, (struct sockaddr*) &clientAddress, &clientLength);
	
	printf("Server has accepted a connection.\n");
	
	while((charsRead = read(clientSocket, buf, sizeof(buf))) > 0) {
		printf("Server recieved: %s\n", buf);
		write(clientSocket, buf, charsRead);
	}
	
	close(clientSocket);
	close(streamSocket);
	
	unlink(serverAddress.sin_addr);
	
	return(0);
}
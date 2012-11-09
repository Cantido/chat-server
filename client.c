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
 
int main() {
	int streamSocket;
	
	struct sockaddr_in serverAddress = { AF_INET, htons(SERVER_PORT) };
	
	int serverLength = sizeof(serverAddress);
	
	struct hostent *hp;
	
	char buf[512];
	int charsRead;
	
	hp = gethostbyname(HOST_NAME);
	
	memcpy((char*) &serverAddress.sin_addr, hp->h_addr_list[0], hp->h_length);
	
	streamSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	connect(streamSocket, (struct sockaddr*) &serverAddress, serverLength);
	
	printf("Client has connected to the server.\n");
	
	while(scanf("%s", buf) != EOF) {
		write(streamSocket, buf, sizeof(buf));
		read(streamSocket, buf, sizeof(buf));
		printf("Recieved from server: %s\n", buf);
	}
	
	close(streamSocket);
	return(0);
}
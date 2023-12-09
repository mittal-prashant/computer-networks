// Client side C program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main(int argc, char const *argv[])
{
	int socket_fd, client_fd, valread;
	struct sockaddr_in server_addr;
	char message[MAX_BUFFER_SIZE];
	char buffer[MAX_BUFFER_SIZE] = {0};

	printf("Enter message for Client 4: ");
	fgets(message, MAX_BUFFER_SIZE, stdin);
	
    // Remove newline character
    message[strcspn(message, "\n")] = '\0';

	// create socket
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return 1;
	}

	// initialize server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) == -1)
	{
		perror("inet_pton");
		return 1;
	}

	// connect to server
	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("connect");
		return 1;
	}

	// send message to server
	if (send(socket_fd, message, strlen(message), 0) == -1)
	{
		perror("send");
		return 1;
	}
	printf("Message sent: %s\n", message);

	// receive response from server
	valread = recv(socket_fd, buffer, MAX_BUFFER_SIZE, 0);
	if (valread == -1)
	{
		perror("recv");
		return 1;
	}
	printf("Response received: %s\n", buffer);

	// close the socket
	close(socket_fd);
	return 0;
}

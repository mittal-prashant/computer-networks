// Server side C program to demonstrate Socket programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[MAX_BUFFER_SIZE] = {0};
	char response[MAX_BUFFER_SIZE] = {0};
	char *message = "Welcome to the server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		perror("Setsockopt");
		exit(EXIT_FAILURE);
	}

	// Assigning address and port to the socket
	memset(&address, '0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Binding the socket to the specified address and port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	// Listening for incoming connections
	if (listen(server_fd, 5) == -1)
	{
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}

	int client_count = 0;
	while (client_count < 5)
	{
		// Accepting incoming connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1)
		{
			perror("Accept failed");
			exit(EXIT_FAILURE);
		}

		printf("New client connected\n");

		// Receiving data from the client
		if ((valread = recv(new_socket, buffer, MAX_BUFFER_SIZE, 0)) == -1)
		{
			perror("Receive failed");
			exit(EXIT_FAILURE);
		}
		printf("Received message from client %d: %s\n", client_count + 1, buffer);

		// Reverse the received input
		int len = strlen(buffer);
		for (int i = 0; i < len; i++)
		{
			response[i] = buffer[len - i - 1];
		}

		// Sending data to the client
		if (send(new_socket, response, strlen(response), 0) == -1)
		{
			perror("Send failed");
			exit(EXIT_FAILURE);
		}
		printf("Response sent to client %d: %s\n", client_count + 1, response);

		// Clearing the buffer
		memset(buffer, 0, sizeof(buffer));

		// Clearing the response
		memset(response, 0, sizeof(response));

		// Closing the connected socket
		close(new_socket);

		client_count++;
	}

	// Closing the listening socket
	close(server_fd);

	return 0;
}

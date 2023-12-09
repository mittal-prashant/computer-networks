// This is a client side C program to demonstrate Socket programming
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
	int socket_fd, valread;
	struct sockaddr_in server_addr;
	char buffer[MAX_BUFFER_SIZE] = {0};
	// create socket
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error: Failed to create socket.");
		return 1;
	}

	// initialize server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) == -1)
	{
		perror("Error: Invalid address or address family not supported.");
		return 1;
	}

	// connect to server
	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Error: Failed to connect to server.");
		return 1;
	}

	// prompt user to enter username
	char username[MAX_BUFFER_SIZE];
	printf("Enter your username (without spaces): ");
	scanf("%s", username);

	// send username to server
	if (send(socket_fd, username, strlen(username), 0) == -1)
	{
		perror("Error: Failed to send username to server.");
		return 1;
	}

	// loop until user exits
	while (1)
	{
		// Get type of ticket from user
		int ticket_type;
		printf("Enter type of ticket (1 for AC, 2 for NON AC, 0 to exit): ");
		scanf("%d", &ticket_type);

		// Send type of ticket to server
		if (send(socket_fd, &ticket_type, sizeof(int), 0) == -1)
		{
			perror("Error: Failed to send ticket type to server.");
			return 1;
		}

		// if user enters 0, exit loop
		if (ticket_type == 0)
		{
			break;
		}

		// Get number of tickets from user
		int num_tickets;
		printf("Enter number of tickets: ");
		scanf("%d", &num_tickets);

		// Send number of tickets to server
		if (send(socket_fd, &num_tickets, sizeof(int), 0) == -1)
		{
			perror("Error: Failed to send number of tickets to server.");
			return 1;
		}

		// receive response from server
		valread = recv(socket_fd, buffer, MAX_BUFFER_SIZE, 0);
		if (valread == -1)
		{
			perror("Error: Failed to receive response from server.");
			return 1;
		}
		printf("%s\n", buffer);
	}

	// close the socket
	close(socket_fd);
	return 0;
}
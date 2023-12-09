// Server side C program to demonstrate Socket programming

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 5

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // mutex lock to synchronize access to shared data
#define AC_TICKET 1
#define NON_AC_TICKET 2
#define EXIT_SYSTEM 0
FILE *fp;							 // file pointer to write logs
int remaining_tickets[2] = {10, 10}; // number of remaining tickets

int num_connections = 5;

void book_tickets(int ticket_type, int num_tickets, const char *username)
{
	remaining_tickets[ticket_type - 1] -= num_tickets;											// decrement number of remaining tickets of given type
	fprintf(fp, "User: %s booked %d tickets of type %d\n", username, num_tickets, ticket_type); // log the booking information
}

// This function is used to handle a client connection
// It takes a void pointer as an argument which is the client socket file descriptor
void *handle_client(void *arg)
{
	// Extracting the client socket file descriptor from the argument
	int client_socket = *(int *)arg;
	free(arg); // Freeing the memory allocated for the argument

	int ticket_type;
	int num_tickets;
	char username[1024];
	char response[1024] = {0};

	// Receiving username of the user from the client
	if (recv(client_socket, username, sizeof(username), 0) <= 0)
	{
		perror("Name receive failed"); // Error message if receive fails
		close(client_socket);
		num_connections--;
		pthread_exit(NULL);
	}

	printf("User %s connected\n", username);

	while (1)
	{
		// Receiving type of tickets from the client
		if (recv(client_socket, &ticket_type, sizeof(int), 0) <= 0)
		{
			perror("Type of tickets receive failed"); // Error message if receive fails
			close(client_socket);
			num_connections--;
			pthread_exit(NULL);
		}

		if (ticket_type == EXIT_SYSTEM)
		{
			printf("User %s disconnected\n", username);
			close(client_socket);
			num_connections--;
			pthread_exit(NULL);
		}

		if (ticket_type != AC_TICKET && ticket_type != NON_AC_TICKET)
		{
			sprintf(response, "Invalid ticket type");
			send(client_socket, response, strlen(response), 0);
			close(client_socket);
			num_connections--;
			pthread_exit(NULL);
		}

		// Receiving number of tickets from the client
		if (recv(client_socket, &num_tickets, sizeof(int), 0) <= 0)
		{
			perror("Number of tickets receive failed"); // Error message if receive fails
			close(client_socket);
			num_connections--;
			pthread_exit(NULL);
		}

		// Checking if the number of tickets is valid
		if (num_tickets <= 0)
		{
			sprintf(response, "Invalid number of tickets");
			send(client_socket, response, strlen(response), 0);
			close(client_socket);
			num_connections--;
			pthread_exit(NULL);
		}

		printf("type: %d, no: %d\n", ticket_type, num_tickets);

		// Acquiring lock before accessing remaining_tickets
		pthread_mutex_lock(&lock);

		// Checking if there are enough tickets available
		if (remaining_tickets[ticket_type - 1] < num_tickets)
		{
			sprintf(response, "Sorry, only %d tickets are available for type %d", remaining_tickets[ticket_type - 1], ticket_type);
			send(client_socket, response, strlen(response), 0);

			// Releasing lock before closing the socket
			pthread_mutex_unlock(&lock);
			close(client_socket);
			num_connections--;
			pthread_exit(NULL);
		}

		// Booking tickets and updating remaining_tickets
		book_tickets(ticket_type, num_tickets, username);
		int remaining = remaining_tickets[ticket_type - 1];

		// Releasing lock after updating remaining_tickets
		pthread_mutex_unlock(&lock);

		// Sending response to the client
		sprintf(response, "%d tickets booked successfully for type %d, %d tickets remaining", num_tickets, ticket_type, remaining);
		send(client_socket, response, strlen(response), 0);
	}

	// Closing the socket and exiting the thread
	close(client_socket);
	num_connections--;
	pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
	char username[20], password[20]; // Declare two character arrays to store the username and password

	printf("Enter your username: "); // Prompt the user to enter their username
	scanf("%s", username);			 // Read the username from the console and store it in the username array

	printf("Enter your password: "); // Prompt the user to enter their password
	scanf("%s", password);			 // Read the password from the console and store it in the password array

	if (strcmp(username, "server") == 0 && strcmp(password, "server") == 0)
	{
		// Use the strcmp function to compare the entered username and password with the hardcoded values
		printf("Login successful!\n"); // If the entered values match, print a success message
	}
	else
	{
		printf("Invalid username or password.\n"); // If the entered values don't match, print an error message
	}

	// Initializing variables
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char response[1024] = {0};

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		perror("Setsockopt failed");
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
	if (listen(server_fd, MAX_CLIENTS) == -1)
	{
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}

	// Opening file for recording server records
	fp = fopen("server_records.txt", "w");
	if (fp == NULL)
	{
		perror("Failed to open server_records.txt");
		exit(EXIT_FAILURE);
	}

	int i = 0;

	// Accepting incoming connections
	while (1)
	{
		if (i == MAX_CLIENTS)
		{
			if (num_connections == 0)
			{
				close(server_fd);
				fclose(fp);
				break;
			}
			continue;
		}

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1)
		{
			perror("Accept failed");
			exit(EXIT_FAILURE);
		}

		pthread_t tid;
		int *pclient = malloc(sizeof(int));
		*pclient = new_socket;

		if (pthread_create(&tid, NULL, handle_client, pclient) != 0)
		{
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}

		pthread_detach(tid);

		i++;
	}

	return 0;
}

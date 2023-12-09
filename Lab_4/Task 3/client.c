#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
	// create client socket
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	char response[1024] = {0};
	char input[1024];

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // create TCP socket
	{
		perror("Socket creation error"); // print error message
		return -1;
	}

	// initialize server address and port
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		perror("Invalid address/ Address not supported");
		return -1;
	}

	// connect to server
	if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
							 sizeof(serv_addr))) < 0)
	{
		perror("Connection failed");
		return -1;
	}

	// keep asking for input until X is entered
	while (1)
	{
		// get postfix expression from user
		printf("Enter postfix expression (or X/x to exit): ");
		fgets(input, 1024, stdin);

		// Remove the newline character if it exists
		if (input[strlen(input) - 1] == '\n')
		{
			input[strlen(input) - 1] = '\0';
		}

		// send expression to server
		send(sock, input, strlen(input), 0);
		if (strcmp(input, "X") == 0 || strcmp(input, "x") == 0)
		{
			break;
		}
		printf("Expression sent to server\n");

		// receive result from server
		valread = read(sock, response, 1024);
		printf("%s\n", response);

		if (response[0] == 'E')
		{
			break;
		}

		// clear buffer and response strings
		memset(buffer, 0, sizeof(buffer));
		memset(response, 0, sizeof(response));
	}

	// close the connected socket
	close(client_fd);
	return 0;
}

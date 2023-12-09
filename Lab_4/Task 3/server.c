#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define PORT 8080

time_t start_time;
FILE *fp;
int num_clients = 0;

void *connection_handler(void *);

int main(int argc, char const *argv[])
{
	start_time = time(NULL);
	fp = fopen("server_logs.txt", "w"); // open the file for writing

	if (fp == NULL)
	{ // check if the file was successfully opened
		printf("Error: Could not open file\n");
		return 1;
	}

	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		pthread_t thread_id;
		if (pthread_create(&thread_id, NULL, connection_handler, (void *)&new_socket) < 0)
		{
			perror("could not create thread");
			return 1;
		}

		num_clients++;
	}

	fclose(fp); // close the file when you're done with it
	return 0;
}

void *connection_handler(void *socket_desc)
{
	int client_id = num_clients;
	int new_socket = *(int *)socket_desc;
	int valread;
	char buffer[1024] = {0};
	char response[1024] = {0};

	// Read the postfix expression from the client
	while (1)
	{
		valread = read(new_socket, buffer, 1024);
		if (buffer[0] == 'X' || buffer[0] == 'x')
		{
			printf("Exiting...\n");
			close(new_socket);

			return NULL;
		}

		printf("Received expression: %s\n", buffer);

		// Evaluate the postfix expression
		double stack[1024];
		int top = -1;
		int i;
		int is_decimal = 0;
		for (i = 0; buffer[i] != '\0'; i++)
		{
			if (buffer[i] == ' ')
				continue;
			if (buffer[i] >= '0' && buffer[i] <= '9')
			{
				double num = 0;
				while (buffer[i] != '\0' && buffer[i] >= '0' && buffer[i] <= '9')
				{
					num = num * 10 + (buffer[i] - '0');
					i++;
				}
				if (buffer[i] == '.') // decimal point found
				{
					is_decimal = 1;
					i++;
					double frac = 0.1;
					while (buffer[i] != '\0' && buffer[i] >= '0' && buffer[i] <= '9')
					{
						num += (buffer[i] - '0') * frac;
						frac /= 10;
						i++;
					}
				}
				stack[++top] = num;
				i--;
			}
			else
			{
				if (top < 1)
				{
					sprintf(response, "Error: Invalid postfix expression: more operators than numbers");
					send(new_socket, response, strlen(response), 0);
					close(new_socket);
					return NULL;
				}
				double op2 = stack[top--];
				double op1 = stack[top--];
				double res;
				switch (buffer[i])
				{
				case '+':
					res = op1 + op2;
					break;
				case '-':
					res = op1 - op2;
					break;
				case '*':
					res = op1 * op2;
					break;
				case '/':
					if (op2 == 0)
					{
						sprintf(response, "Error: Division by zero error");
						send(new_socket, response, strlen(response), 0);
						close(new_socket);
						return NULL;
					}
					res = op1 / op2;
					break;
				default:
					sprintf(response, "Error: Invalid operator: %c", buffer[i]);
					send(new_socket, response, strlen(response), 0);
					close(new_socket);
					return NULL;
				}
				stack[++top] = res;
			}
		}
		if (top != 0)
		{
			sprintf(response, "Error: Invalid postfix expression: more numbers than operators");
			send(new_socket, response, strlen(response), 0);
			close(new_socket);
			return NULL;
		}
		double result = stack[top];

		time_t end_time = time(NULL);
		double time_elapsed = difftime(end_time, start_time);

		if ((int)result == result && !is_decimal)
		{
			sprintf(response, "Result: %d", (int)result);

			fprintf(fp, "%d %s %d %d\n", client_id, buffer, (int)result, (int)time_elapsed);
			fflush(fp);
		}
		else
		{
			sprintf(response, "Result: %.4f", result);

			fprintf(fp, "%d %s %.4f %d\n", client_id, buffer, result, (int)time_elapsed);
			fflush(fp);
		}
		send(new_socket, response, strlen(response), 0);
		printf("Sent response: %s\n", response);

		memset(buffer, 0, sizeof(buffer));
		memset(response, 0, sizeof(response));
	}
}

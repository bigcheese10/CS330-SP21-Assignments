/*
 Program: client.c
 Author: Kaleb Ringenberg
*/

// imports baby
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <err.h>

// constant
#define SEND_BUFFER_SIZE 2048

int client(char* router_ip, char* router_port, char* server_ip, char* server_port, char* message)
{

	// declare variables
	int sock_fd;
	int temp_sockfd;
	int error;
	int accept_it = 1;

	struct addrinfo hints, *serv_info, *x;
	struct sockaddr_in client_addr;

	socklen_t sin_size;

	char* status;
	char ip_addr[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	// don't need the local host this time

	error = getaddrinfo(router_ip, router_port, &hints, &serv_info);
	if (error)
	{
		errx(1, "%s", gai_strerror(error));
	}

	if ((sock_fd = socket(serv_info->ai_family, serv_info->ai_socktype,
		serv_info->ai_protocol)) < 0)
	{
		perror("Server: Unable to plug into socket.");
		exit(-1);
	}

	if (connect(sock_fd, serv_info->ai_addr, serv_info->ai_addrlen) < 0)
	{
		perror("Server: Unable to plug into socket.");
		exit(-1);
	}

	char msg_builder[RECV_BUFFER_SIZE] = "";
	strncat(msg_builder, "-", 2);
	// give it the server_ip
	strncat(msg_builder, server_ip, strlen(server_ip));
	strncat(msg_builder, "-", 2);
	// give it the server_port
	strncat(msg_builder, server_port, strlen(server_port));
	strncat(msg_builder, "-", 2);
	// give it the router_ip
	strncat(msg_builder, router_ip, strlen(router_ip));
	strncat(msg_builder, "-", 2);
	// give it the router_port
	strncat(msg_builder, router_port, strlen(router_port));
	// give it the message
	strncat(message, msg_builder, strlen(msg_builder));

	if (send(sock_fd, message, (strlen(message)), 0) < 0)
	{
		perror("Client: error sending information.");
		exit(-1);
	}

	memset(message, 0, sizeof message);
	close(sock_fd);

	return 0;
}
/*
 * main()
 * Parse command-line arguments and call client function
*/
int main(int argc, char** argv) 
{
    char* server_ip;
    char* server_port;
    char* router_ip;
    char* router_port;
    if (argc != 5) 
	{
        fprintf(stderr, "Usage: ./client.c [router IP] [router port] [server IP] [server port] < [message]");
        exit(EXIT_FAILURE);
    }

    router_ip = argv[1];
    router_port = argv[2];
    server_ip = argv[3];
    server_port = argv[4];
    return client(router_ip, router_port, server_ip, server_port);
}

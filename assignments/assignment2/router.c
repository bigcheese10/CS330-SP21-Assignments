/*
Program: router.c
Author: Kaleb Ringenberg
*/

// imports baby
#include <arpa/inet.h> 
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// constants
#define QUEUE_LENGTH 10
#define RECV_BUFFER_SIZE 2048

//routing table
struct forwarding_table 
{
	char ip_addr[16];
	char port[5];
};

//prototypes
int client(char* router_ip, char* router_port, char* server_ip, char* server_port, char* message);
int router(char* router_port) 
{
	// variables
	int sockfd, new_sockfd, newer_sockfd;
	int size;
	int accept_it = 1;

	struct addr_info hints, * serv_info, * x;
	struct sockaddr_in client_addr; 

	socklen_t length;
	               
	char ip_addr[INET_ADDRSTRLEN]; 
	unsigned char buffer[RECV_BUFFER_SIZE], serialized_buffer[RECV_BUFFER_SIZE];
	const char* delim = "-";
	char* temp_holder[5];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	// uses local host
	hints.ai_flags = AI_PASSIVE; 

	error = getaddrinfo(NULL, router_port, &hints, &serv_info);

	if (error) 
	{
		errx(1, "%s", gai_strerror(error));
	}
	
	if ((sockfd = socket(serv_info->ai_family, serv_info->ai_socktype,
		serv_info->ai_protocol)) < 0) 
	{
		perror("Router: Unable to plug into socket.");
		exit(-1);
	}
	
	// again for the reason that we are using the same machine for all of these
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &accept_it, sizeof(int)) == -1) 
	{
		perror("setsockopt failed to be used correctly.");
		exit(-1);
	}
	
	if (bind(sockfd, serv_info->ai_addr, serv_info->ai_addrlen) < 0)
	{
		close(sockfd);
		perror("Router: Unable to plug into socket.");
		exit(-1);
	}

	freeaddrinfo(serv_info);
	if (serv_info == NULL) 
	{
		fprintf(stderr, "Router: Unable to plug into socket.");
		exit(-1);
	}
	
	if (listen(sockfd, QUEUE_LENGTH) < 0)
	{
		perror("Router: Unable to plug into socket.");
		exit(-1);
	}
	while (1) 
	{
		length = sizeof client_addr;

		if ((new_sockfd = accept(sockfd, (struct sockaddr*)&client_addr,
			&length)) < 0)
		{
			perror("Router: connection rejected.");
			exit(-1); 
		}
	
		while ((size = recv(new_sockfd, buffer, RECV_BUFFER_SIZE, 0)) > 0) 
		{
			char* breaker = strtok(buffer, delim);
			int i = 0;
			while (breaker != NULL)
			{
				temp_holder[i] = breaker;
				i++;
				breaker = strtok(NULL, delim);
			}
			memset(buffer, 0, sizeof buffer);
		}
		close(new_sockfd);
	}
	return 0;
}

int client(char* router_ip, char* router_port, char* server_ip, char* server_port, char* message)
{

	// declare variables
	int sock_fd;
	int temp_sockfd;
	int error;
	int accept_it = 1; 

	struct addrinfo hints, * serv_info, * x;
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

int main(int argc, char** argv)
{
	struct forwarding_table fwd_table[10];
	struct forwarding_table first; 
	struct forwarding_table second;

	strcpy(first.ip_addr, "192.0.2.6");
	strcpy(first.port, "8080");
	strcpy(second.ip_addr, "155.232.60.19");
	strcpy(second.port, "8081");

	fwd_table[0] = first;
	fwd_table[1] = second;

	
	if (argc != 2) 
	{
		fprintf(stderr, "Usage: ./router.c [router port]");
		exit(EXIT_FAILURE);
	}

	char* router_port;
	router_port = argv[1];
	return router(router_port);
}
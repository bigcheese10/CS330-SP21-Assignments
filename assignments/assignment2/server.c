/*
Program: server.c
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


void *get_in_addr(struct sockaddr *sock_a) 
{
  if (sock_a->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sock_a)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sock_a)->sin6_addr);
}


int server(char *server_port, char *gateway_ip, char *gateway_port) 
{

  char* temp_holder[5]; 
  char* delim = "-";
  char ip_addr[INET_ADDRSTRLEN];
  unsigned char buffer[RECV_BUFFER_SIZE], serialized_buffer[RECV_BUFFER_SIZE];

  struct addrinfo hints, *serv_info, *x;
  struct sockaddr_in client_addr; 

  int error;
  int sock_fd;
  int temp_sockfd;
  int size;
  int accept_it = 1;                  
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // uses host machine IP
  hints.ai_flags = AI_PASSIVE; 

  error = getaddrinfo(NULL, server_port, &hints, &serv_info);
  if (error)
  {
    errx(1, "%s", gai_strerror(error));
  }

  // socket up
  if ((sock_fd = socket(serv_info->ai_family, serv_info->ai_socktype,
                       serv_info->ai_protocol)) < 0) 
  {
    perror("Server: Unable to plug into socket.");
    exit(-1);
  }

  
  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &accept_it, sizeof(int)) == -1)
  {
	// since we're doing this all on one machine, the IP is already in use so the compiler goes
	// "what the heck" but this fixes this.
    perror("setsockopt failed to be used correctly.");
    exit(-1);
  }

  if (bind(sock_fd, serv_info->ai_addr, serv_info->ai_addrlen) < 0) 
  {
    close(sock_fd);
    perror("Server: Unable to plug into socket.");
    exit(-1);
  }

  // another check to see if the server connects correctly or not
  freeaddrinfo(serv_info);
  if (serv_info == NULL)
  {
    fprintf(stderr, "Server: Unable to plug into socket.");
    exit(-1);
  }

  if (listen(sock_fd, QUEUE_LENGTH) < 0) 
  {
    perror("Server: Unable to establish connection.");
    exit(-1);
  }

 
  while (1) 
  {
    length = sizeof client_addr;
    if ((temp_sockfd = accept(sock_fd, (struct sockaddr *)
		&client_addr, &length)) < 0)
	{
      perror("Server: Denied connection.");
      exit(-1); 
    }

    while((size= recv(temp_sockfd, buffer, RECV_BUFFER_SIZE, 0)) > 0) 
    {
        // okay so since we had to tie our message all together, this basically unties everything
        char* breaker = strtok(buffer, delim);
        int i = 0;
        while (breaker != NULL)
        {
            temp_holder[i] = breaker;
            i++;
            breaker = strtok(NULL, delim);
        }
        write(1, temp_holder[0], strlen(temp_holder[0]));

      // checking for port and IPs
        if ( strcmp(server_port,temp_holder[2])!=0 && strcmp(gateway_ip,temp_holder[3])!=0 )
        {
           perror("Server: IP/Port mixup, disconnecting.");
           exit(-1);
        }   
    }
    fflush(stdout);
    close(temp_sockfd);
  }
  return 0;
}

int main(int argc, char **argv)
{
  char *server_port;
  char *router_ip;
  char *router_port;

  if (argc != 4) 
  {
    fprintf(stderr, "Usage: ./server.c [server port] [gateway_ip] [gateway_port]");
    exit(EXIT_FAILURE);
  }

  server_port = argv[1];
  router_ip = argv[2];
  router_port = argv[3];
  return server(server_port, router_ip, router_port);
}

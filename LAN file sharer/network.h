#ifndef NET_H
#define NET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define SERVERPORT 3490
#define BACKLOG 1

int init_broadcast_socket(struct sockaddr_in *their_addr);

int init_broadcaster_socket();

int init_tcp_connection(struct sockaddr_in *their_addr);

int sendall(char *buf, size_t len, int sockfd);

void *get_in_addr(struct sockaddr *sa);

int init_socket(int socktype);

int handle_connection(struct sockaddr_storage *their_addr, int sockfd);

int recv_all(int sockfd, void *buf, size_t len);

void recv_and_save_file(size_t host_file_size, int new_sockfd, FILE *fp);

#endif
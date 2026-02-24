#ifndef TRANSFER_H
#define TRANSFER_H

#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define HOST_NAME_MAX 256

char *construct_message(char *filepath);

int handle_response(char *buf);

char *construct_header(char *filepath, uint32_t *header_size);

char *construct_response_packet(char response);

int handle_discovery(struct sockaddr_storage *their_addr, char *buf, int sockfd);


#endif
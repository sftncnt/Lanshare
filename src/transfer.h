#ifndef TRANSFER_H
#define TRANSFER_H

#include <stdint.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>


#define HOST_NAME_MAX 256

char *construct_message(char *filepath, size_t *message_len);

int handle_response(char *buf);

char *construct_header(char *filepath, uint32_t *header_size);

char *construct_response_packet(char response, size_t *packet_size);

int handle_discovery(char **response_to_send, size_t *packet_size, char *buf, FILE **fp);


#endif

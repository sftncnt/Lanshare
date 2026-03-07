#include "transfer.h"
#include "network.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

char *construct_message(char *filepath, size_t *message_len) {
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        return NULL;
    }
    hostname[sizeof(hostname) - 1] = '\0';
    char *filename = get_file_name(filepath);
    uint16_t hostname_len = strlen(hostname);
    uint16_t net_hostname_len = htons(hostname_len);
    char *combined_message = malloc(strlen(hostname) + strlen(filename) + 3);
    if (!combined_message) {
        return NULL;
    }

    memcpy(combined_message, &net_hostname_len, 2);
    memcpy(combined_message + 2, hostname, hostname_len);
    memcpy(combined_message + 2 + hostname_len, filename, strlen(filename) + 1);

    *message_len = hostname_len + 2 + strlen(filename) + 1;
    
    return combined_message;

}

int handle_response(char *buf) {
    uint16_t net_len;
    uint16_t hostname_len;
    char response;

    memcpy(&response, &buf[2], 1);
    
    if (response == 'n') {
        printf("\nConnection rejected\n");
        return -1;
    } 
    
    memcpy(&net_len, buf, 2);
    hostname_len = ntohs(net_len);
    char their_hostname[hostname_len + 1];
    memcpy(their_hostname, &buf[3], hostname_len);
    their_hostname[hostname_len] = '\0';
    printf("\n%s accepted your request\n", their_hostname);
    return 0;
    
}

char *construct_header(char *filepath, uint32_t *header_size) {
    char *filename = get_file_name(filepath);
    size_t filename_size = strlen(filename);
    uint32_t net_filename_size = htonl((uint32_t)(filename_size));

    off_t file_size = get_file_size(filepath);
    uint32_t net_file_size = htonl((uint32_t)(file_size));

    *header_size = ((sizeof(uint32_t) * 2) + filename_size);
    char *header = malloc(*header_size);
    if (!header) {
        return NULL;
    }

    memcpy(header, &net_filename_size, 4);
    memcpy(header + 4, &net_file_size, 4);
    memcpy(header + 8, filename, filename_size);

    return header;
}

// receiver functions

char *construct_response_packet(char response, size_t *packet_size) {
    char hostname[HOST_NAME_MAX];
    if ((gethostname(hostname, sizeof hostname)) == -1) {
        perror("gethostname");
        return NULL;
    }
    size_t hostname_len = strnlen(hostname, sizeof hostname);
    uint16_t net_len = htons((uint16_t)(hostname_len));
    *packet_size = hostname_len + 2 + 1;

    char *packet = malloc(*packet_size);
    if (!packet) {
        return NULL;
    }
    memcpy(packet, &net_len, 2);

    packet[2] = response;

    memcpy(packet + 3, hostname, hostname_len);

    return packet;
}

int handle_discovery(char **response_to_send, size_t *packet_size, char *buf, FILE **fp, char *filepath) {
    uint16_t net_hostname_len;
    memcpy(&net_hostname_len, buf, 2);
    uint16_t host_hostname_len = ntohs(net_hostname_len);
    char hostname[host_hostname_len + 1];
    memcpy(hostname, buf + 2, host_hostname_len);
    hostname[host_hostname_len] = '\0';

    uint16_t filename_size = strlen(buf + 5);
    char filename[filename_size + 1];
    memcpy(filename, buf + 2 + host_hostname_len, filename_size + 1);
    filename[filename_size] = '\0';
    
    printf("%s would like to share %s\n", hostname, filename);
    int gettingInput = 1;
    while (gettingInput) {
        printf("Would you like to receive this file? (y/n): ");
        char response = getchar();
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        if (response == 'y') {
            *response_to_send = construct_response_packet(response, packet_size);
            *fp = open_file(filename, filepath);
            return 1;
        } else if (response == 'n') {
            *response_to_send = construct_response_packet(response, packet_size);
            return 0;
        } else {
            printf("Please enter either y or n\n");
        }
        
    }

    return 0;

}

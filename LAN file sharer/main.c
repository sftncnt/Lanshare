#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "network.h"
#include "transfer.h"
#include "utils.h"

void usage() {
    printf("usage: send <filepath>\n");
    printf("usage: receive <directory>\n");
}

int send_mode(char *filepath) {
    int sockfd;
    struct sockaddr_in their_addr;
    int rv;
    int numbytes;
    off_t file_size;
    char buf[1024];

    FILE *file_to_share = get_file(filepath);
    if (!file_to_share) {
        cleanup_and_exit(NULL, 0);
    }

    file_size = get_file_size(filepath);
    if (file_size == -1) {
        cleanup_and_exit(file_to_share, sockfd);
    }

    sockfd = init_broadcaster_socket();
    if (sockfd == -1) {
        cleanup_and_exit(file_to_share, sockfd);
    }
    
    rv = init_broadcast_socket(&their_addr);
    if (rv == -1) {
        cleanup_and_exit(file_to_share, sockfd);
    }

    char *first_message = construct_message(filepath);
    if (!first_message) {
        cleanup_and_exit(file_to_share, sockfd);
    }

    numbytes = sendto(sockfd, first_message, strlen(first_message) + 1, 0, (struct sockaddr *)&their_addr, sizeof their_addr);

    if (numbytes == -1) {
        perror("sendto");
        return 1;
    }

    printf("sent %d bytes to %s\n", numbytes,
     inet_ntoa(their_addr.sin_addr));
    
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    printf("Address length is %d\n", addr_len);
    printf("Waiting for response...");
    fflush(stdout);
    
    numbytes = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&their_addr, &addr_len);

    if (numbytes == -1) {
        perror("recvfrom");
        return 1;
    }

    close(sockfd);

    rv = handle_response(buf);
    if (rv == -1) {
        cleanup_and_exit(file_to_share, sockfd);
    }

    sockfd = init_tcp_connection(&their_addr);
    if (sockfd == -1) {
        cleanup_and_exit(file_to_share, sockfd);
    }

    uint32_t header_size;
    char *header = construct_header(filepath, &header_size);
    if (!header) {
        cleanup_and_exit(file_to_share, sockfd);
    }
    printf("header_size: %d\n", header_size);
    rv = sendall(header, header_size, sockfd);
    if (rv == -1) {
        cleanup_and_exit(file_to_share, sockfd);
    }

    char chunk[BUFSIZ];
    int bytes_read;
    long total = 0;
    while ((bytes_read = fread(chunk, 1, BUFSIZ, file_to_share)) > 0) {
        rv = sendall(chunk, bytes_read, sockfd);
        if (rv == -1) {
            cleanup_and_exit(file_to_share, sockfd);
        }
        total += bytes_read;
        double progress = (total * 100.0) / file_size;
        printf("\rSending progress: %.2f%%", progress);
        fflush(stdout);
    }
    printf("\n");
    fclose(file_to_share);
    close(sockfd);
    return 0;
}

int receive_mode() {
    int sockfd, numbytes;
    struct sockaddr_storage their_addr; // connector's address info
    socklen_t sin_size;
    char buf[BUFSIZ];
    

    sockfd = init_socket(SOCK_DGRAM);
    if (sockfd == -1) {
        cleanup_and_exit(NULL, sockfd);
    }
    
    printf("listener: waiting to recvfrom...\n");

    sin_size = sizeof their_addr;

    if ((numbytes = recvfrom(sockfd, buf, BUFSIZ-1, 0, (struct sockaddr *)&their_addr,
            &sin_size)) == -1) {
        perror("recvfrom");
        return 1;
    }

    if (handle_discovery(&their_addr, buf, sockfd) == 0) {
        printf("Rejected connetion\n");
        return 0;
    } 

    close(sockfd);

    sockfd = init_socket(SOCK_STREAM);
    if (sockfd == -1) {
        cleanup_and_exit(NULL, sockfd);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        cleanup_and_exit(NULL, sockfd);
    }

    int new_sockfd = handle_connection(&their_addr, sockfd);
    if (new_sockfd == -1) {
        cleanup_and_exit(NULL, sockfd);
    }
    close(sockfd);

    uint32_t net_filename_size;
    uint32_t net_file_size;

    if(recv_all(new_sockfd, &net_filename_size, sizeof net_filename_size) == -1) {
        cleanup_and_exit(NULL, new_sockfd);
    }

    if(recv_all(new_sockfd, &net_file_size, sizeof net_file_size) == -1) {
        cleanup_and_exit(NULL, new_sockfd);
    }

    uint32_t host_filename_size = ntohl(net_filename_size);
    uint32_t host_file_size = ntohl(net_file_size);

    printf("Filename size: %u, File size: %u\n",
        host_filename_size, host_file_size);
    printf("Raw net filename: %u\n", net_filename_size);
    printf("Raw net filesize: %u\n", net_file_size);

    char *filename = malloc(host_filename_size + 1);
    if (!filename) {
        perror("malloc");
        cleanup_and_exit(NULL, new_sockfd);
    }

    if (recv_all(new_sockfd, filename, host_filename_size) == -1) {
        cleanup_and_exit(NULL, new_sockfd);
    }
    filename[host_filename_size] = '\0';

    printf("Filename: %s\n", filename);
    
    
    FILE *fp = open_file(filename);
    free(filename);
    recv_and_save_file(host_file_size, new_sockfd, fp);
    fclose(fp);
    close(new_sockfd);



    
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }
    if (strcmp(argv[1], "send") == 0) {
        if (argc != 3) {
            usage();
            return 1;
        }
        return send_mode(argv[2]);
    }
    if (strcmp(argv[1], "receive") == 0) {
        return receive_mode();
    }
    usage();
    return 1;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>

#define SERVERPORT 3490	// the port users will be connecting to
#define HOST_NAME_MAX 256
#define FILE_SIZE_MAX 4000000000
#define CHUNK_SIZE 8192

struct header_format {
    uint32_t filename_size;
    uint32_t file_size;
    char *filename;
};

struct sockaddr_in init_broadcast_socket() {
    struct sockaddr_in their_addr;
    struct hostent *he;
    char *broadcast_address = "255.255.255.255";
    if ((he = gethostbyname(broadcast_address)) == NULL) {
        perror("gethostbyname\n");
        exit(1);
    } else {
        printf("got host\n");
    }
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(SERVERPORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
    return their_addr;
}

int init_broadcaster_socket() {
    struct sockaddr_in my_addr;
    int sockfd;
    int broadcast = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
     if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
        perror("sets");
    }
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERVERPORT);
    memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero);
    bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr);

    return sockfd;
}

FILE *get_file(char *filepath) {
    printf("%s\n", filepath);
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("open");
        exit(1);
    }
    return fp;
}

char *get_file_name(char *filepath) {
    char *name = strrchr(filepath, '/');
    if (name != NULL) {
        name++;
    } else {
        return filepath;
    }
    printf("Name: %s\n", name);
    return name;

}

char *construct_message(char *filepath) {
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        exit(1);
    }
    hostname[sizeof(hostname) - 1] = '\0';
    char *filename = get_file_name(filepath);
    char *message = " would like to share ";
    char *combined_message = malloc(strlen(hostname) + strlen(message) + strlen(filename) + 1);
    strcpy(combined_message, hostname);
    strcat(combined_message, message);
    strcat(combined_message, filename);
    printf("Combined message size: %s\n", combined_message);
    return combined_message;

}

int handle_response(char *buf) {
    uint16_t net_len;
    uint16_t hostname_len;
    char response;

    memcpy(&response, &buf[2], 1);
    if (response == 'n') {
        printf("Connection rejected\n");
        exit(1);
    } 
    
    memcpy(&net_len, buf, 2);
    hostname_len = ntohs(net_len);
    char their_hostname[hostname_len + 1];
    memcpy(their_hostname, &buf[3], hostname_len);
    their_hostname[hostname_len] = '\0';
    printf("%s accepted your request\n", their_hostname);
    return 1;
    
}

int init_tcp_connection(struct sockaddr_in *their_addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("tcp socket");
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr *)their_addr, sizeof *their_addr) == -1) {
        perror("tcp connection");
        exit(1);
    }
    return sockfd;

}

off_t get_file_size(char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        exit(1);
    }
    if (st.st_size > FILE_SIZE_MAX) {
        printf("File size too large");
        exit(1);
    }
    return st.st_size;
}

char *construct_header(char *filepath, uint32_t *header_size) {
    char *filename = get_file_name(filepath);
    size_t filename_size = strlen(filename);
    uint32_t net_filename_size = htonl((uint32_t)(filename_size));

    off_t file_size = get_file_size(filepath);
    printf("File size: %ld\n", file_size);
    uint32_t net_file_size = htonl((uint32_t)(file_size));

    *header_size = ((sizeof(uint32_t) * 2) + filename_size);
    printf("Header size: %ls\n", header_size);
    char *header = malloc(*header_size);

    memcpy(header, &net_filename_size, 4);
    memcpy(header + 4, &net_file_size, 4);
    memcpy(header + 8, filename, filename_size);

    return header;
}

int sendall(char *buf, int *len, int sockfd) {
    int total = 0;
    int bytes_left = *len;
    int n;
    while(total < *len) {
        n = send(sockfd, buf, bytes_left, 0);
        if (n == -1) {
            perror("send");
            exit(1);
        }
        total += n;
        bytes_left -= n;
    } 
    *len = total;
    return n == -1? -1: 0;
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in their_addr;
    int numbytes;
    char buf[1024];


    if (argc != 2) {
        fprintf(stderr, "usage: filepath\n");
        exit(1);
    }

    FILE *file_to_share = get_file(argv[1]);

    sockfd = init_broadcaster_socket();
    their_addr = init_broadcast_socket();

    char *first_message = construct_message(argv[1]);

    numbytes = sendto(sockfd, first_message, strlen(first_message) + 1, 0, (struct sockaddr *)&their_addr, sizeof their_addr);

    if (numbytes == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes to %s\n", numbytes,
     inet_ntoa(their_addr.sin_addr));
    
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    printf("Address length is %d\n", addr_len);
    //memset(&their_addr.sin_addr, 0, sizeof their_addr.sin_addr);
    numbytes = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&their_addr, &addr_len);

    if (numbytes == -1) {
        perror("recvfrom");
        exit(1);
    }

    close(sockfd);

    handle_response(buf);
    sockfd = init_tcp_connection(&their_addr);

    uint32_t header_size;
    char *header = construct_header(argv[1], &header_size);
    printf("header_size: %d\n", header_size);
    int send_status = sendall(header, &header_size, sockfd);

    char chunk[8192];
    int chunk_size = 8192;
    int bytes_read;
    while ((bytes_read = fread(chunk, 1, chunk_size, file_to_share)) > 0) {
        //printf("chunks: %s\n", chunk);
        sendall(chunk, &bytes_read, sockfd);
    }


    return 0;


}
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>


int init_broadcast_socket(struct sockaddr_in *their_addr) {
    struct hostent *he;
    char *broadcast_address = "255.255.255.255";
    if ((he = gethostbyname(broadcast_address)) == NULL) {
        perror("gethostbyname\n");
        return -1;
    } else {
        printf("got host\n");
    }
    their_addr->sin_family = AF_INET;
    their_addr->sin_port = htons(SERVERPORT);
    their_addr->sin_addr = *((struct in_addr *)he->h_addr_list[0]);
    memset(their_addr->sin_zero, '\0', sizeof their_addr->sin_zero);
    return 0;
}

int init_broadcaster_socket() {
    struct sockaddr_in my_addr;
    int sockfd;
    int broadcast = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return -1;
    }
     if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
        perror("setsockopt");
        return -1;
    }
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERVERPORT);
    memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero);
    bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr);

    return sockfd;
}

int init_tcp_connection(struct sockaddr_in *their_addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("tcp socket");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *)their_addr, sizeof *their_addr) == -1) {
        perror("tcp connection");
        return -1;
    }
    return sockfd;

}

int sendall(char *buf, size_t len, int sockfd) {
    int total = 0;
    int bytes_left = len;
    int n;
    while(total < len) {
        n = send(sockfd, buf, bytes_left, 0);
        if (n == -1) {
            perror("send");
            return -1;
        }
        total += n;
        bytes_left -= n;
    } 
    
    return n == -1? -1: 0;
}

// Receiver functions

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int init_socket(int socktype) {
    struct addrinfo hints, *servinfo, *p;
    int sockfd, rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    hints.ai_flags = AI_PASSIVE; // use my IP

    char port[5];
    sprintf(port, "%d", SERVERPORT);
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket\n");
            continue;
        }

        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: bind\n");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL) {
        fprintf(stderr, "Server failed to bind\n");
        return -1;
    }

    return sockfd;
}

int handle_connection(struct sockaddr_storage *their_addr, int sockfd) {
    socklen_t sin_size = sizeof *their_addr;
    
    int new_sockfd = accept(sockfd, (struct sockaddr *)their_addr, &sin_size);
    if (new_sockfd == -1) {
        perror("tcp connection");
        return -1;
    }
    
    return new_sockfd;

}

int recv_all(int sockfd, void *buf, size_t len) {
    int total = 0;
    int n;
    printf("Length passed: %ld\n", len);
    while (total < len)
    {
        n = recv(sockfd, buf + total, len, 0);
        if (n == -1) {
            perror("recv");
            return -1;
        }
        total += n;
    }
    return total;
}

void recv_and_save_file(size_t host_file_size, int new_sockfd, FILE *fp) {
    char buffer[8192];
    size_t total = 0;

    while (total < host_file_size) {
        size_t to_read = host_file_size - total;
        if (to_read > sizeof(buffer))
            to_read = sizeof(buffer);

        ssize_t n = recv(new_sockfd, buffer, to_read, 0);
        if (n < 0) {
            perror("recv");
            break;
        }
        
        fwrite(buffer, 1, n, fp);
        total += n;
        double progress = (total * 100.0) / host_file_size;
        printf("\rDownload progress: %.2f%%", progress);
        fflush(stdout);
    }
    printf("\n");
}
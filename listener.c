#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define BACKLOG 10
#define MAXBUFLEN 100
#define HOST_NAME_LEN 255

struct response_packet {
    uint16_t name_len;   // network byte order
    uint8_t  decision;  // 'y' or 'n'
    char     hostname[HOST_NAME_LEN]; // not null-terminated
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char *construct_response_packet(char response) {
    char hostname[HOST_NAME_LEN];
    gethostname(hostname, sizeof hostname);
    size_t hostname_len = strnlen(hostname, sizeof hostname);
    uint16_t net_len = htons((uint16_t)(hostname_len));
    size_t packet_size = hostname_len + 2 + 1;

    char *packet = malloc(packet_size);
    memcpy(packet, &net_len, 2);

    packet[2] = response;

    memcpy(packet + 3, hostname, hostname_len);

    return packet;
}

int init_listener() {
    struct addrinfo hints, *servinfo, *p;
    int sockfd, numbytes, rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket\n");
            continue;
        }
        
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
        exit(1);
    }

    return sockfd;
}

int main() {
    int sockfd, numbytes;
    struct sockaddr_storage their_addr; // connector's address info
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    char buf[MAXBUFLEN];
    

    sockfd = init_listener();
    
    
    printf("listener: waiting to recvfrom...\n");

    sin_size = sizeof their_addr;

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr,
            &sin_size)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, 
            sizeof s);
    printf("listener got packet from: %s\n", s);
    printf("listener: packet is %d bytes long\n", numbytes);
    printf("listener: packet contains \"%s\"\n", buf);
    int gettingInput = 1;
    while (gettingInput) {
        printf("Would you like to receive this file? (y/n): ");
        char response = getchar();
        printf("Your response is %c\n", response);
        if (response == 'y' || response == 'n') {
            char *response_to_send = construct_response_packet(response);
            printf("Packet: %s\n", response_to_send);
            numbytes = sendto(sockfd, response_to_send, sizeof response_to_send, 0, (struct sockaddr *)&their_addr, 
                sizeof their_addr);
            if (numbytes == -1) {
                perror("sendto");
                exit(1);
            }
            free(response_to_send);
            gettingInput = 0;
        } else {
            printf("Please enter either y or n\n");
        }
    }
    printf("Sent %d bytes\n", numbytes);
    close(sockfd);
    return 0;
    
        
       
}
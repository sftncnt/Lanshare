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

#define SERVERPORT 3490	// the port users will be connecting to

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

int main(int argc, char *argv[]) {
    int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr;
    struct sockaddr_in their_addr2;
    int numbytes;
    int broadcast = 1;
    char buf[1024];
    char name[1024];
    gethostname(name, strlen(name));
    char *message = " would like to share a file";
    char *combined_message = malloc(strlen(name) + strlen(message) + 1);
    strcpy(combined_message, name);
    strcat(combined_message, message);


    /*if (argc != 3) {
        fprintf(stderr, "usage: broadcaster hostname message");
    }

    if ((he = gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname\n");
        exit(1);
    } else {
        printf("got host\n");
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
        perror("sets");
    }*/

    /*struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3490);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(my_addr.sin_zero, 0, sizeof my_addr.sin_zero);

    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(SERVERPORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);*/

    sockfd = init_broadcaster_socket();
    their_addr = init_broadcast_socket();

    numbytes = sendto(sockfd, combined_message, strlen(combined_message), 0, (struct sockaddr *)&their_addr, sizeof their_addr);

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
    printf("Received %d bytes from %s\n", numbytes, inet_ntoa(their_addr.sin_addr));
    uint16_t net_len;
    memcpy(&net_len, buf, 2);
    uint16_t hostname_len = ntohs(net_len);
    printf("Message: %d\n", hostname_len);
    
    

    close(sockfd);

    return 0;


}
#include "headers.h"

#define PORT "4950"
#define MAXBUFLEN 512

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int init() {
    int sockfd;
    struct  addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        return 2;
    }

    freeaddrinfo(servinfo);

    return sockfd;
    
}

int main() {
    char s[INET6_ADDRSTRLEN];
    char buf[MAXBUFLEN];
    int numbytes;
    socklen_t addr_len;
    struct sockaddr_storage their_addr;
    int sockfd;

    sockfd = init();

    printf("listener: waiting to recvfrom \n");

    addr_len =  sizeof their_addr;

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvform");
        exit(1);
    }

    printf("listner: got packet from %s \n",
        inet_ntop(their_addr.ss_family, get_in_addr((struct  sockaddr *)&their_addr),
        s, sizeof s));
    
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);

    close(sockfd);

    return 0;

}
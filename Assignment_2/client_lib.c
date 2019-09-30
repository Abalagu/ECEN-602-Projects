#include "client_lib.h"

// len for payload length
sbcp_msg_t make_msg_join(char *username, size_t len)
{
    sbcp_attribute_t *sbcp_attr_join = malloc(sizeof(sbcp_attribute_t) + len);
    sbcp_attr_join->sbcp_attribute_type = USERNAME;
    memcpy(sbcp_attr_join->payload, username, len);
    sbcp_attr_join->len = len + 4; //payload + 2 + 2

    sbcp_msg_t *msg_join = malloc(sbcp_attr_join->len + sizeof(*sbcp_attr_join));
    memcpy(msg_join->sbcp_attributes, sbcp_attr_join, sbcp_attr_join->len);
    // msg_join->sbcp_attributes = sbcp_attr_join;
    msg_join->vrsn_type_len = (VRSN << 23 | JOIN << 16 | sbcp_attr_join->len + 4);

    return *msg_join;
}

int writen(int sockfd, char *buf, size_t size_buf)
{
    int numbytes;
    while ((numbytes = send(sockfd, buf, size_buf, 0)) == -1 && errno == EINTR)
    {
        // manually restarting
        continue;
    }
    return numbytes;
}

int readline(int sockfd, char *recvbuf)
{
    int numbytes;
    while ((numbytes = recv(sockfd, recvbuf, MAXDATASIZE - 1, 0)) == -1 && errno == EINTR)
    {
        // manually restarting
    }
    return numbytes;
}

int server_lookup_connect(char *host, char *server_port)
{
    struct addrinfo hints, *server_info, *p;
    int status;
    int sock_fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE;     // fill in my IP

    // argv[1]: IPAdr
    // argv[2]: Port
    if ((status = getaddrinfo(host, server_port, &hints, &server_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -2;
    }

    for (p = server_info; p != NULL; p = p->ai_next)
    { //loop through link list
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd == -1)
        { //socket creation failed
            perror("client: socket");
            continue;
        }

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1)
        { //connection failed
            close(sock_fd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return -2;
    }

    printf("client: connected to %s:%s\n", host, server_port);

    freeaddrinfo(server_info);
    return sock_fd;
}
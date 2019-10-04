#include "client_lib.h"

sbcp_msg_t make_msg_join(char *username, size_t name_len)
{
    sbcp_msg_t msg_join = {0};
    msg_join.vrsn_type_len = (VRSN << 23 | JOIN << 16 | sizeof(sbcp_msg_t));

    // fill in message part
    msg_join.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
    msg_join.sbcp_attributes[0].len = name_len;
    memcpy(msg_join.sbcp_attributes[0].payload, username, name_len);

    return msg_join;
}

sbcp_msg_t make_msg_send(char *message, size_t msg_len)
{
    sbcp_msg_t msg_send = {0};
    msg_send.vrsn_type_len = (VRSN << 23 | SEND << 16 | sizeof(sbcp_msg_t));

    // fill in message part
    msg_send.sbcp_attributes[0].sbcp_attribute_type = MESSAGE;
    msg_send.sbcp_attributes[0].len = msg_len;
    memcpy(msg_send.sbcp_attributes[0].payload, message, msg_len);

    return msg_send;
}

// idle message from client to server
sbcp_msg_t make_msg_idle_c(char *username, size_t name_len)
{
    sbcp_msg_t msg_idle = {0};
    msg_idle.vrsn_type_len = (VRSN << 23 | IDLE << 16 | sizeof(sbcp_msg_t));

    // fill in username
    msg_idle.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
    msg_idle.sbcp_attributes[0].len = name_len;
    memcpy(msg_idle.sbcp_attributes[0].payload, username, name_len);

    return msg_idle;
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
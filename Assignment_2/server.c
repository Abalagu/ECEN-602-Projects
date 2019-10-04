#include "server.h"

#define BACKLOG 10
#define MAXDATASIZE 1500

// function taken from beej's guide
void sigchild_handler(int s)
{
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int server_read(int new_fd, char *buf)
{
    // read the received buffer from the socket
    return recv(new_fd, buf, MAXDATASIZE - 1, 0);
}

int server_write(int new_fd, char *buf)
{
    // send the buffer to the socket
    return send(new_fd, buf, MAXDATASIZE - 1, 0);
}

int main(int argc, char *argv[])
{

    int sockfd, new_fd;
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t addr_size;
    int yes = 1;
    struct sigaction sa;
    int status, numbytes;
    char buf[MAXDATASIZE];

    if (argc != 2)
    {
        // check for correct usage
        fprintf(stderr, "usage: echos Port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP

    if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    // loop through all the results and bind to the first correct
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        // allow other sockets to bind to this port
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
        {
            perror("setsocketopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    // we don't need it now
    freeaddrinfo(res);

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    // reap all dead process - function taken from beej's guide
    sa.sa_handler = sigchild_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections....\n");

    int sin_size = sizeof(their_addr);
    char str[sin_size];
    while (1)
    {
        while ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1 && errno == EINTR)
        {
            // manually restart
            continue;
        }

        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), str, sin_size);
        printf("server: got conection from %s\n", str);

        if (!fork())
        {
            while (numbytes = server_read(new_fd, buf))
            {
                if (numbytes == -1 && errno == EINTR)
                {
                    continue;
                }

                if (numbytes == -1)
                {
                    perror("recv");
                    exit(1);
                }

                if (numbytes == 0)
                {
                    printf("TCP FIN received");
                }

                // struct exchange test: server
                printf("recv size: %d\n", numbytes);
                sbcp_msg_t *msg = (sbcp_msg_t *)buf;
                parse_msg_join(*msg);
                sbcp_msg_t msg_ack = make_msg_ack(1, "luming");
                // printf("recv: %s\n", buf);
                memcpy(buf, &msg_ack, sizeof(msg_ack));
                printf("sent ACK\n");
                // echo back
                while ((numbytes = server_write(new_fd, buf) == -1 && errno == EINTR))
                {
                    continue;
                }

                if (numbytes == -1)
                {
                    perror("send");
                    exit(1);
                }

                // printf("send: %s\n", buf);
            }
            // client disconnected
            printf("TCP FIN received");
            fflush(stdout);
            // let it go
            close(new_fd);
        }
    }
    return 0;
}

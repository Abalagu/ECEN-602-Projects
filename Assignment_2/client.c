#include "client.h"

int main(int argc, char *argv[])
{
    printf("\n");
    char username[] = "luming";
    char message[] = "hello world!";
    sbcp_msg_t msg_join = make_msg_join(username, sizeof(username));
    // sbcp_msg_t msg_join = make_msg_join(message, sizeof(message), username, sizeof(username));
    parse_msg_join(msg_join);

    // early return to skip socket
    // return 0;

    // make simple connection to server
    int sock_fd;
    char *host = "localhost", *server_port = "12345";
    sock_fd = server_lookup_connect(host, server_port);
    if(sock_fd < 0){
        printf("connection error.\n");
        return 1;
    }
    printf("connected\n");

    // struct exchange test: client
    char buf[MAXDATASIZE];
    char recv_buf[MAXDATASIZE];
    // printf("sizeof msg join: %ld\n", sizeof(msg_join));
    memcpy(buf, &msg_join, sizeof(msg_join));
    writen(sock_fd, buf, sizeof(msg_join));
    readline(sock_fd, recv_buf);
    sbcp_msg_t *msg = (sbcp_msg_t*)recv_buf;
    parse_msg_ack(*msg);
    // printf("sizeof buf: %ld\n", sizeof(buf));
    // printf("sizeof msg: %ld\n", sizeof(msg_join));
    
    while(1){

    }
    return 0;
}

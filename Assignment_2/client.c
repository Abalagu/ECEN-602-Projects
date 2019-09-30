#include "client.h"

int main(int argc, char *argv[])
{
    printf("\n");
    sbcp_msg_t msg_join = make_msg_join("luming");
    parse_msg_join(msg_join);

    // make simple connection to server
    int sock_fd;
    char *host = "localhost", *server_port = "12345";
    sock_fd = server_lookup_connect(host, server_port);
    printf("connected\n");
    writen(sock_fd, "hello");
    
    while(1){

    }
    return 0;
}

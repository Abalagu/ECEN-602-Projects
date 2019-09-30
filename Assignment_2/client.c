#include "client.h"

int main(int argc, char *argv[])
{
    printf("\n");
    char username[] = "lumingxuabalguasdasdsa";
    sbcp_msg_t msg_join = make_msg_join(username, sizeof(username));
    int msg_len = parse_msg_join(msg_join);
    printf("msg len: %d\n", msg_len);

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
    memcpy(buf, &msg_join, msg_len);
    writen(sock_fd, buf, msg_len);
    // printf("sizeof buf: %ld\n", sizeof(buf));
    // printf("sizeof msg: %ld\n", sizeof(msg_join));
    
    while(1){

    }
    return 0;
}

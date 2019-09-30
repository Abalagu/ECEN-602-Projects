#ifndef CLIENT_LIB_H_
#define CLIENT_LIB_H_

#include "common_lib.h"
#include "config.h"

#define MAXDATASIZE 100

#endif



sbcp_msg_t make_msg_join(char* username);

// int writen(int sockfd, char *buf);
int writen(int sockfd, char *buf, size_t size_buf);

int readline(int sockfd, char *recvbuf);

int server_lookup_connect(char *host, char *server_port);


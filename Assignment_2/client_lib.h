#ifndef CLIENT_LIB_H_
#define CLIENT_LIB_H_

#include "common_lib.h"
#include "config.h"

#define MAXDATASIZE 1500

#endif

sbcp_msg_t make_msg_fwd(char *message, size_t msg_len, char *username, size_t name_len);

sbcp_msg_t make_msg_join(char *username, size_t payload_len);

// int writen(int sockfd, char *buf);
int writen(int sockfd, char *buf, size_t size_buf);

int readline(int sockfd, char *recvbuf);

int server_lookup_connect(char *host, char *server_port);

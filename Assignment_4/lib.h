#ifndef LIB_H_
#define LIB_H_

#include "headers.h"

typedef enum http_err_t {
  HTTP_OK = 0,
  HTTP_FAIL = -1,
} http_err_t;

int server_lookup_connect(char *host, char *server_port);

int writen(int sockfd, char *buf, size_t size_buf);

int readline(int sockfd, char *recvbuf);

void print_hex(void *array, size_t len);

int accept_client(int sockfd);

http_err_t server_init(char *port, int *sockfd);

void sigchld_handler(int s);

#endif

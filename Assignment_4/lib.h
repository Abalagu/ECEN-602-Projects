#ifndef LIB_H_
#define LIB_H_

#include "headers.h"

typedef enum http_err_t {
  HTTP_OK = 0,
  HTTP_FAIL = -1,
} http_err_t;

typedef struct {
  char host[253]; // max size of a domain name
  // randomly assigned, need a fix
  char path[1500];
  // these for testing
  char user_agent[10];
  char connection[10];
} request_t;

typedef struct {
  // add fields you need to check
  char status[20];
  char content_length[10];
  char date[30];
  // etc, etc
} response_t;

http_err_t server_lookup_connect(char *host, char *server_port, int *sock_fd);

int written(int sockfd, char *buf, size_t size_buf);

int readline(int sockfd, char *recvbuf);

void print_hex(void *array, size_t len);

http_err_t accept_client(int listen_fd, int *client_fd);

http_err_t server_init(char *port, int *sockfd);

void sigchld_handler(int s);

void parse_request(char req_buf[1500], request_t *req);

void parse_response(char res_buf[1500], response_t *res);

#endif

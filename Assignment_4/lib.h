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

// --- BEGIN LRU CACHE MANAGEMENT ---
typedef enum node_status_t {
  IN_USE = 1,
  NOT_IN_USE = 2,
} node_status_t;

typedef struct cache_node_t {
  struct cache_node_t *prev, *next;
  char *buffer;
  size_t buffer_size;
  node_status_t status;
} cache_node_t;

typedef struct cache_queue_t {
  int max_slot;
  cache_node_t *front, *rear;
} cache_queue_t;

cache_node_t *new_cache_node(cache_node_t *prev, cache_node_t *next,
                             size_t buffer_size);

cache_queue_t *new_cache_queue(size_t max_slot);

void free_cache_node(cache_node_t **cache_node);

void free_cache_queue(cache_queue_t **cache_queue);

void cache_enqueue(cache_queue_t *cache_queue, cache_node_t *new_node);

void print_cache_node(cache_node_t *cache_node);

void print_cache_queue(cache_queue_t *cache_queue);

// --- END LRU CACHE MANAGEMENT

// --- BEGIN FD MANAGEMENT ---
typedef enum fd_type_t {
  DUMMY = 0,
  LISTEN = 1,
  CLIENT = 2,
  SERVER = 3,
} fd_type_t;

typedef struct fd_node_t {
  struct fd_node_t *prev, *next;
  int fd;
  fd_type_t type;
  cache_node_t *cache_node;
} fd_node_t;

typedef struct fd_list_t {
  fd_node_t *front, *rear;
  int max_client;
} fd_list_t;

fd_node_t *new_fd_node(fd_node_t *prev, fd_node_t *next, int fd, fd_type_t type,
                       cache_node_t *cache_node);

fd_list_t *new_fd_list(int max_client);

void fd_list_append(fd_list_t *fd_list, fd_node_t *new_node);

void fd_list_remove(fd_node_t *fd_node);

void free_fd_node(fd_node_t **fd_node);

void free_fd_list(fd_list_t **fd_queue);

void print_fd_node(fd_node_t *fd_node);

void print_fd_list(fd_list_t *fd_queue);

// --- END FD MANAGEMENT ---

http_err_t server_lookup_connect(char *host, char *server_port, int *sock_fd);

int written(int sockfd, char *buf, size_t size_buf);

int readline(int sockfd, char *recvbuf);

void print_hex(void *array, size_t len);

http_err_t accept_client(int listen_fd, int *client_fd);

http_err_t server_init(char *port, int *sockfd);

void parse_request(char req_buf[1500], request_t *req);

void parse_response(char res_buf[1500], response_t *res);

#endif

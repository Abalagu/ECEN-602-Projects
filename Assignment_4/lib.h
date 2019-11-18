#ifndef LIB_H_
#define LIB_H_

#include "headers.h"

typedef enum http_err_t {
  HTTP_OK = 0,
  HTTP_FAIL = -1,
} http_err_t;

typedef enum node_status_t {
  IN_USE = 1,  // for lru cache
  IDLE = 2,    // for lru cache // indicate operation complete
  READING = 3, // for socket select
  WRITING = 4, // for socket select
} node_status_t;

// --- BEGIN LRU CACHE MANAGEMENT ---

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
  struct fd_node_t *prev, *next, *proxied;
  int fd;
  fd_type_t type;
  node_status_t status;
  cache_node_t *cache_node;
  off_t offset; // record partial read/write progress
} fd_node_t;

typedef struct fd_list_t {
  fd_node_t *front, *rear;
  int max_client;
} fd_list_t;

fd_node_t *new_fd_node(fd_node_t *prev, fd_node_t *next, int fd, fd_type_t type,
                       node_status_t status, cache_node_t *cache_node);

fd_list_t *new_fd_list(int max_client);

void fd_list_append(fd_list_t *fd_list, fd_node_t *new_node);

void fd_list_remove(fd_node_t *fd_node);

void free_fd_node(fd_node_t **fd_node);

void free_fd_list(fd_list_t **fd_list);

void print_fd_node(fd_node_t *fd_node);

void print_fd_list(fd_list_t *fd_list);

// --- END FD MANAGEMENT ---

// --- BEGIN SOCKET UTIL ---
http_err_t server_lookup_connect(char *host, char *server_port, int *sock_fd);

int written(int sockfd, char *buf, size_t size_buf);

int readline(int sockfd, char *recvbuf, size_t read_size);

void print_hex(void *array, size_t len);

http_err_t accept_client(int listen_fd, int *client_fd);

http_err_t server_init(char *port, int *sockfd);

int get_max_fd(fd_list_t *fd_list);

int fd_select(fd_list_t *fd_list, fd_set *read_fds, fd_set *write_fds);

int cache_recv(fd_node_t *fd_node);

int cache_send(fd_node_t *fd_node);

http_err_t listen_fd_handler(fd_list_t *fd_list, fd_node_t *fd_node);
http_err_t client_read_handler(fd_list_t *fd_list, fd_node_t *fd_node);
http_err_t client_write_handler(fd_list_t *fd_list, fd_node_t *fd_node);
http_err_t server_read_handler(fd_list_t *fd_list, fd_node_t *fd_node,
                               cache_queue_t *cache_queue);
http_err_t server_write_handler(fd_list_t *fd_list, fd_node_t *fd_node);
// --- END SOCKET UTIL ---

// --- BEGIN UNIT TEST FUNCTIONS ---
void cache_init_test();
void cache_enqueue_test();
void cache_eviction_test();
void fd_list_test();
// --- END UNIT TEST FUNCTIONS ---

#endif

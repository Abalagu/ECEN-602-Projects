
#include "lib.h"

// given sockfd, return local port
http_err_t get_sock_port(int sockfd, int *local_port) {
  struct sockaddr_in sin;
  int addrlen = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &addrlen) == 0 &&
      sin.sin_family == AF_INET && addrlen == sizeof(sin)) {
    *local_port = ntohs(sin.sin_port);
    return HTTP_OK;
  } else {
    printf("find port error.\n");
    return HTTP_FAIL;
  }
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

http_err_t server_lookup_connect(char *host, char *server_port, int *sock_fd) {
  struct addrinfo hints, *server_info, *p;
  int status;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // argv[1]: IPAdr
  // argv[2]: Port
  if ((status = getaddrinfo(host, server_port, &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return HTTP_FAIL;
  }

  for (p = server_info; p != NULL; p = p->ai_next) { // loop through link list
    *sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (*sock_fd == -1) { // socket creation failed
      perror("client: socket");
      continue;
    }

    if (connect(*sock_fd, p->ai_addr, p->ai_addrlen) ==
        -1) { // connection failed
      close(*sock_fd);
      perror("client: connect");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return HTTP_FAIL;
  }

  printf("connected to %s:%s\n", host, server_port);

  freeaddrinfo(server_info);
  return HTTP_OK;
}

int written(int sockfd, char *buf, size_t size_buf) {
  int numbytes;
  while ((numbytes = send(sockfd, buf, size_buf, 0)) == -1 && errno == EINTR) {
    // manually restarting
    continue;
  }
  return numbytes;
}

int readline(int sockfd, char *recvbuf) {
  int numbytes;
  while ((numbytes = recv(sockfd, recvbuf, MAX_DATA_SIZE - 1, 0)) == -1 &&
         errno == EINTR) {
    // manually restarting
  }
  return numbytes;
}

// print buffer of given length in hexadecimal form, prefix with 0
void print_hex(void *array, size_t len) {
  char *parray = array;
  for (int i = 0; i < len; i++) {
    printf(" %02x", parray[i]);
  }
  printf("\n");
}

// given listening socket, accept possible client
http_err_t accept_client(int listen_fd, int *client_fd) {
  struct sockaddr_storage their_addr; // connector's address information

  int sin_size = sizeof(their_addr);
  char str[sin_size];
  *client_fd = accept(listen_fd, (struct sockaddr *)&their_addr, &sin_size);

  if (*client_fd < 0) {
    perror("accept_client");
    return HTTP_FAIL;
  }

  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
            str, sin_size);

  printf("server: got conection from %s\n", str);
  return HTTP_OK;
}

// launch STREAM socket at given port. port = "" results in ephemeral port.
http_err_t server_init(char *port, int *sockfd) {
  int rv, numbytes;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return HTTP_FAIL;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("listener: socket");
      continue;
    }
    if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }
  if (p == NULL) {
    return HTTP_FAIL;
  }
  listen(*sockfd, BACKLOG);
  freeaddrinfo(servinfo);
  int local_port;
  get_sock_port(*sockfd, &local_port);
  printf("HTTP Proxy Server Port: %d\n", local_port);
  return HTTP_OK;
}

// --- BEGIN LRU CACHE MANAGEMENT
cache_node_t *new_cache_node(cache_node_t *prev, cache_node_t *next,
                             size_t buffer_size) {
  cache_node_t *cache_node = calloc(1, sizeof(cache_node_t));
  // designated initializer in C99
  // https://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99
  *cache_node = (cache_node_t){
      .prev = prev,
      .next = next,
      .buffer = NULL,
      .buffer_size = buffer_size,
      .status = NOT_IN_USE,
  };
  if (buffer_size != 0) {
    // use calloc instead of malloc to both allocate and init to 0
    cache_node->buffer = calloc(buffer_size, sizeof(char));
  }
  return cache_node;
}

cache_queue_t *new_cache_queue(size_t max_slot) {
  cache_queue_t *cache_queue = calloc(1, sizeof(cache_queue_t));
  *cache_queue = (cache_queue_t){
      .max_slot = max_slot,
      .front = new_cache_node(NULL, NULL, 0), // create a node for head
      .rear = NULL,
  };

  cache_node_t *prev_node = cache_queue->front, *new_node = NULL;
  for (int i = 0; i < max_slot - 1; i++) { // one created during queue init
    new_node = new_cache_node(prev_node, NULL, 0);
    if (new_node->prev != NULL) {
      // link .next field of the previous node to itself.
      new_node->prev->next = new_node;
    }
    prev_node = new_node; // becomes the prev node in the next loop
  }
  cache_queue->rear = new_node;

  return cache_queue;
}

void free_cache_node(cache_node_t **cache_node) {
  if (*cache_node == NULL) { // avoid double free
    return;
  }
  // to set pointer to NULL, one would need to pass pointer of the pointer
  if ((*cache_node)->buffer != NULL) {
    free((*cache_node)->buffer);
    (*cache_node)->buffer = NULL;
  }
  free((*cache_node));
  *cache_node = NULL;
  return;
}

void free_cache_queue(cache_queue_t **cache_queue) {
  if (*cache_queue == NULL) { // avoid double free
    return;
  }
  cache_node_t *cache_node = (*cache_queue)->front, *next = NULL;
  while (cache_node != NULL) {
    next = cache_node->next; // temp store next node before memory free
    free_cache_node(&cache_node);
    cache_node = next;
  }
  free(*cache_queue);
  *cache_queue = NULL;
  return;
}

void print_cache_node(cache_node_t *cache_node) {
  printf("buffer size: %ld ", cache_node->buffer_size);
  printf("buffer: %s ", cache_node->buffer == NULL ? "NULL" : "NOT NULL");
  printf("status: %d ", cache_node->status);
  printf("prev: %s ", cache_node->prev == NULL ? "NULL" : "NOT NULL");
  printf("next: %s\n", cache_node->next == NULL ? "NULL" : "NOT NULL");
  return;
}

void print_cache_queue(cache_queue_t *cache_queue) {
  if (cache_queue == NULL) {
    printf("cache queue is NULL\n");
    return;
  }
  cache_node_t *cache_node = cache_queue->front;
  while (cache_node != NULL) {
    print_cache_node(cache_node);
    cache_node = cache_node->next;
  }
  return;
}

// evict the last node of the queue, not exposed
void cache_dequeue(cache_queue_t *cache_queue) {
  // store the last but one node
  cache_node_t *tmp_node = cache_queue->rear->prev;
  cache_queue->rear->prev = NULL; // evicted, null its prev and next
  cache_queue->rear->next = NULL; // by default rear.next is NULL
  // if NOT_IN_USE, free the memory
  if (cache_queue->rear->status == NOT_IN_USE) {
    free_cache_node(&cache_queue->rear);
  }
  // if IN_USE, let the using process free its memory
  cache_queue->rear = tmp_node; // the last but one becomes the last node
  cache_queue->rear->next = NULL;

  return;
}

// always dequeue before enqueue, thus maintain a constant queue size
void cache_enqueue(cache_queue_t *cache_queue, cache_node_t *new_node) {
  cache_dequeue(cache_queue);
  cache_queue->front->prev = new_node;
  new_node->next = cache_queue->front;
  new_node->prev = NULL;
  cache_queue->front = new_node;
  return;
}
// --- END LRU CACHE MANAGEMENT

// --- BEGIN FD MANAGEMENT ---
fd_node_t *new_fd_node(fd_node_t *prev, fd_node_t *next, int fd, fd_type_t type,
                       cache_node_t *cache_node) {
  fd_node_t *new_node = calloc(1, sizeof(fd_node_t));
  *new_node = (fd_node_t){
      .prev = prev,
      .next = next,
      .fd = fd,
      .type = type,
      .cache_node = cache_node,
  };
  return new_node;
}

// create two dummy nodes to simplify append and remove
fd_list_t *new_fd_list(int max_client) {
  fd_list_t *fd_list = calloc(1, sizeof(fd_list_t));
  fd_node_t *fd_front_dummy = new_fd_node(NULL, NULL, -1, DUMMY, NULL);
  fd_node_t *fd_rear_dummy = new_fd_node(NULL, NULL, -1, DUMMY, NULL);
  fd_front_dummy->next = fd_rear_dummy;
  fd_rear_dummy->prev = fd_front_dummy;

  *fd_list = (fd_list_t){
      .front = fd_front_dummy,
      .rear = fd_rear_dummy,
      .max_client = max_client,
  };
  return fd_list;
}

void free_fd_node(fd_node_t **fd_node) {
  if (*fd_node != NULL) {
    free(*fd_node);
    *fd_node = NULL;
  }
  return;
}

int count_client(fd_list_t *fd_list) {
  int count = 0;
  fd_node_t *next = fd_list->front;
  while (next != NULL) {
    if (next->type = CLIENT) {
      count += 1;
    }
  }
  return count;
}

// append before rear of the linked list
void fd_list_append(fd_list_t *fd_list, fd_node_t *new_node) {
  new_node->prev = fd_list->rear->prev;
  new_node->next = fd_list->rear;
  fd_list->rear->prev->next = new_node;
  fd_list->rear->prev = new_node;
  return;
}

// doubly linked list, concat prev and next to remove itself
void fd_list_remove(fd_node_t *fd_node) {
  fd_node->prev->next = fd_node->next;
  fd_node->next->prev = fd_node->prev;
  fd_node->prev = NULL;
  fd_node->next = NULL;
  return;
}

void print_fd_node(fd_node_t *fd_node) {
  printf("prev: %s ", fd_node->prev == NULL ? "NULL" : "NOT");
  printf("next: %s ", fd_node->next == NULL ? "NULL" : "NOT");
  printf("fd: %d ", fd_node->fd);
  printf("type: %d ", fd_node->type);
  printf("cache node: %s\n", fd_node->cache_node == NULL ? "NULL" : "NOT");
  return;
}

void print_fd_list(fd_list_t *fd_list) {
  if (fd_list == NULL) {
    printf("fd queue is NULL\n");
    return;
  }
  int count = 0;
  fd_node_t *fd_node = fd_list->front;
  while (fd_node != NULL) {
    print_fd_node(fd_node);
    count += 1;
    fd_node = fd_node->next;
  }
  printf("%d fdnode(s) printed\n", count);
  return;
}

// --- END FD MANAGEMENT ---

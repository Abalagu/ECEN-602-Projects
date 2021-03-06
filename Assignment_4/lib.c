
#include "lib.h"

//--- PARSING FUNCTIONS ---
void parse_request(char req_buf[1500], http_info_t *req) {
  // char url[1500]= "GET /somefile HTTP/1.0\r\nHost: www.go.com\r\nUser-Agent:
  // Team4\r\n\r\n"; fix size
  char buf[1500] = {0};

  // separate the first line which contains the path
  char *loc = strchr(req_buf, '\r');
  int pos = (loc == NULL ? -1 : loc - req_buf);

  // First 4 and last 13 contain standard HTTP version info, not needed
  int _pos = pos - 13;
  strncpy(req->path, &req_buf[4], _pos);

  // rest of the header
  memcpy(buf, &(req_buf[pos + 2]), 1500 * sizeof(char));

  char *token;
  char *_buf = buf;
  char *key = malloc(strlen(buf));
  char *value = malloc(strlen(buf));

  // \r\n is the delim here according to spec
  while ((token = strtok_r(_buf, "\r\n", &_buf))) {
    memset(key, 0, strlen(buf));
    memset(value, 0, strlen(buf));
    // printf("token:%s\n", token);
    loc = strchr(token, ' ');
    pos = (loc == NULL ? -1 : loc - token);

    strncpy(key, token, pos);
    strncpy(value, &(token[pos + 1]), strlen(token));

    if (!strcmp(key, "Host:")) {
      strcpy(req->host, value);
    } else if (!strcmp(key, "User-Agent:")) {
      strcpy(req->user_agent, value);
    } else if (!strcmp(key, "Connection:")) {
      strcpy(req->connection, value);
    }
  }
  free(key);
  free(value);
}

void parse_response(char res_buf[1500], http_info_t *res) {
  char buf[1500] = {0};

  char *loc = strchr(res_buf, '\r');
  int pos = (loc == NULL ? -1 : loc - res_buf);

  // status code does not begin until 9
  memcpy(res->status, &(res_buf[9]), pos - 9);

  // copy rest of the buffer
  memcpy(buf, &(res_buf[pos + 2]), 1500 * sizeof(char));
  if (DEBUG)
    printf("buf:%s\n", buf);

  char *token;
  char *_buf = buf;
  char *key = malloc(strlen(buf));
  char *value = malloc(strlen(buf));

  // the delim is \r\n
  while ((token = strtok_r(_buf, "\r\n", &_buf))) {
    // printf("token:%s\n", token);
    loc = strchr(token, ' ');
    pos = (loc == NULL ? -1 : loc - token);

    if (pos < 0) {
      // EOF
      break;
    }
    memset(key, 0, strlen(buf));
    memset(value, 0, strlen(buf));

    strncpy(key, token, pos);
    strncpy(value, &(token[pos + 1]), strlen(token));

    if (!strcmp(key, "Content-Length:")) {
      strcpy(res->content_length, value);
    } else if (!strcmp(key, "Date:")) {
      strcpy(res->date, value);
    }
    // add more fields which we are interested in
  }

  token = strstr(res_buf, "\r\n\r\n");
  // printf("\n\n  RESPONSE HEADER:\n%.*s", (int)(token - res_buf), res_buf);
  free(key);
  free(value);
}
//
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

  printf("\nproxy: conection from %s\n", str);
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
  http_info_t *http_info = calloc(1, sizeof(http_info_t));
  // designated initializer in C99
  // https://stackoverflow.com/questions/7265583/combine-designated-initializers-and-malloc-in-c99
  *cache_node = (cache_node_t){
      .prev = prev,
      .next = next,
      .buffer = NULL,
      .buffer_size = buffer_size,
      .status = IDLE,
      .http_info = http_info,
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
  printf("host: %s%s\n", cache_node->http_info->host,
         cache_node->http_info->path);
  printf("date: %s\n", cache_node->http_info->date);

  // printf("buffer size: %ld ", cache_node->buffer_size);
  // printf("buffer: %s ", cache_node->buffer == NULL ? "NULL" : "NOTN");
  // printf("status: %d ", cache_node->status);
  // printf("prev: %s ", cache_node->prev == NULL ? "NULL" : "NOTN");
  // printf("next: %s\n", cache_node->next == NULL ? "NULL" : "NOTN");
  return;
}

void print_cache_queue(cache_queue_t *cache_queue) {
  if (cache_queue == NULL) {
    printf("cache queue is NULL\n");
    return;
  }
  cache_node_t *cache_node = cache_queue->front;
  printf("  CURRENT LRU CACHE QUEUE:\n");
  int count = 0;
  while (cache_node != NULL) {
    if (cache_node->http_info->info_complete) {
      printf("\ncache slot %d:\n", count);
      print_cache_node(cache_node);
      count += 1;
    }
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
  // if IDLE, free the memory
  if (cache_queue->rear->status == IDLE) {
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

cache_node_t *is_cache_hit(cache_queue_t *cache_queue, http_info_t *http_info) {
  cache_node_t *node = cache_queue->front;
  while (node != NULL) {
    if (node->http_info->info_complete) {
      if (strcmp(node->http_info->host, http_info->host) == 0 &&
          strcmp(node->http_info->path, http_info->path) == 0) {
        // same url resource, cache hit
        return node;
      }
    }
    node = node->next;
  }
  return NULL;
}
// --- END LRU CACHE MANAGEMENT

// --- BEGIN FD MANAGEMENT ---
fd_node_t *new_fd_node(fd_node_t *prev, fd_node_t *next, int fd, fd_type_t type,
                       node_status_t status, cache_node_t *cache_node) {
  fd_node_t *new_node = calloc(1, sizeof(fd_node_t));
  *new_node = (fd_node_t){
      .prev = prev,
      .next = next,
      .proxied = NULL, // associate client and server nodes
      .fd = fd,
      .type = type,
      .status = status,
      .cache_node = cache_node,
      .cache_node_backup = NULL,
      .offset = 0,
      .flag = 0,
  };
  return new_node;
}

// create two dummy nodes to simplify append and remove
fd_list_t *new_fd_list(int max_client) {
  fd_list_t *fd_list = calloc(1, sizeof(fd_list_t));
  fd_node_t *fd_front_dummy = new_fd_node(NULL, NULL, -1, DUMMY, IDLE, NULL);
  fd_node_t *fd_rear_dummy = new_fd_node(NULL, NULL, -1, DUMMY, IDLE, NULL);
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
  if (fd_node->type == 2) {
    printf("client node, request url: %s\n",
           fd_node->cache_node->http_info->path);
  } else if (fd_node->type == 3) {
    printf("server node, reequest url: %s\n",
           fd_node->cache_node->http_info->path);
  }

  // printf("prev: %s ", fd_node->prev == NULL ? "NULL" : "NOTN");
  // printf("next: %s ", fd_node->next == NULL ? "NULL" : "NOTN");
  // printf("fd: %d ", fd_node->fd);
  // printf("type: %d ", fd_node->type);
  // printf("status: %d ", fd_node->status);
  // printf("cache node: %s ", fd_node->cache_node == NULL ? "NULL" : "NOTN");
  // printf("offset: %ld\n", fd_node->offset);
  return;
}

void print_fd_list(fd_list_t *fd_list) {
  if (fd_list == NULL) {
    printf("fd queue is NULL\n");
    return;
  }
  int count = 0;
  printf(" Current Connection: \n");
  fd_node_t *fd_node = fd_list->front;
  while (fd_node != NULL) {
    print_fd_node(fd_node);
    count += 1;
    fd_node = fd_node->next;
  }
  // printf("%d fdnode(s) printed\n", count);
  return;
}

// --- END FD MANAGEMENT ---

// --- BEGIN SOCKET UTIL ---
int get_max_fd(fd_list_t *fd_list) {
  int fd_max = 0;
  fd_node_t *next = fd_list->front;
  while (next != NULL) {
    fd_max = next->fd > fd_max ? next->fd : fd_max;
    next = next->next;
  }
  return fd_max;
}

// select call based on fd status, pass select retval to outside
int fd_select(fd_list_t *fd_list, fd_set *read_fds, fd_set *write_fds) {
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  FD_ZERO(read_fds);
  FD_ZERO(write_fds);
  fd_node_t *node = fd_list->front;
  while (node != NULL) {
    if (node->status == READING) {
      FD_SET(node->fd, read_fds);
    } else if (node->status == WRITING) {
      FD_SET(node->fd, write_fds);
    }
    node = node->next;
  }
  return select(get_max_fd(fd_list) + 1, read_fds, write_fds, NULL, &tv);
}

int written(int sockfd, char *buf, size_t size_buf) {
  int numbytes;
  while ((numbytes = send(sockfd, buf, size_buf, 0)) == -1 && errno == EINTR) {
    // manually restarting
    continue;
  }
  return numbytes;
}

int readline(int sockfd, char *recvbuf, size_t read_size) {
  int numbytes;
  while ((numbytes = recv(sockfd, recvbuf, read_size, 0)) == -1 &&
         errno == EINTR) {
    // manually restarting
  }
  return numbytes;
}

void print_http_info(http_info_t *http_info) {
  printf("\n\n  HEADER INFO: \n");
  printf("status: %s\n", http_info->status);
  printf("host: %s\n", http_info->host);
  printf("path: %s\n", http_info->path);
  printf("date: %s\n", http_info->date);
  printf("content-length: %s\n", http_info->content_length);
}

// read to recvbuf, with specified offset
int cache_recv(fd_node_t *fd_node) {
  int numbytes;
  char buf_recv[MAX_DATA_SIZE] = {0};
  while ((numbytes = recv(fd_node->fd, buf_recv, MAX_DATA_SIZE, 0)) == -1 &&
         errno == EINTR) {
    // manually restarting
  }
  // reallocate for cache node buffer if exceeds free space
  if (fd_node->cache_node->buffer_size - fd_node->offset <= numbytes) {
    fd_node->cache_node->buffer =
        realloc(fd_node->cache_node->buffer,
                fd_node->cache_node->buffer_size + MAX_DATA_SIZE);
    fd_node->cache_node->buffer_size += MAX_DATA_SIZE;
  }

  // copy recv to cache_node, increment offset
  memcpy(fd_node->cache_node->buffer + fd_node->offset, buf_recv, numbytes);
  fd_node->offset += numbytes;

  // change fd_node status if recv complete
  // TODO: it's an unreliable inference of client write complete
  // cannot receiving 0 bytes as client is waiting for response
  if (fd_node->type == CLIENT) {
    fd_node->status = IDLE;
    fd_node->offset = 0;
  }

  // can receive 0 byte from server
  if (fd_node->type == SERVER) {
    if (fd_node->cache_node->http_info->info_complete == 0) {
      parse_response(buf_recv, fd_node->cache_node->http_info);
      if (fd_node->flag == 1) { // if it's conditional get
        if (strstr(fd_node->cache_node->http_info->status, "304") != NULL) {
          printf("Conditional GET return: 304\n");
          // print_http_info(fd_node->cache_node->http_info);
          // 304, NOT MODIFIED
          // update cache time in
          strcpy(fd_node->cache_node_backup->http_info->date,
                 fd_node->cache_node->http_info->date);
          // swap backup cache hit node to cache node
          fd_node->proxied->cache_node = fd_node->cache_node_backup;
          close(fd_node->fd);
          fd_node->status = IDLE;
          fd_node->offset = 0; // reset offset to head
        }
      }
      // toggle flag, prevent later no parsing
      fd_node->cache_node->http_info->info_complete = 1;
      // fill in header fields from response
      print_http_info(fd_node->cache_node->http_info);
    }

    if (numbytes == 0) {
      close(fd_node->fd);
      fd_node->status = IDLE;
      fd_node->offset = 0; // reset offset to head
    }
  }

  return numbytes;
}

int cache_send(fd_node_t *fd_node) {
  int numbytes;
  int remain_size = fd_node->cache_node->buffer_size - fd_node->offset;
  // send at most MAX_DATA_SIZE in one go
  int send_size = remain_size > MAX_DATA_SIZE ? MAX_DATA_SIZE : remain_size;
  char *buffer = fd_node->cache_node->buffer + fd_node->offset;
  char conditional_get[500] = {0};

  if (fd_node->type == SERVER) {
    if (fd_node->flag == 1) { // send conditional get
      sprintf(conditional_get,
              "GET %s HTTP/1.0\r\nHost: %s\r\nIf-Modified-Since: %s\r\n\r\n",
              fd_node->cache_node_backup->http_info->path,
              fd_node->cache_node_backup->http_info->host,
              fd_node->cache_node_backup->http_info->date);
      buffer = conditional_get;
      // send_size = strlen(conditional_get);
    }
  } // else, send normal get

  while ((numbytes = send(fd_node->fd, buffer, send_size, 0)) == -1 &&
         errno == EINTR) {
    // manually restarting
    continue;
  }
  // record offset by actual sent amount
  fd_node->offset += numbytes;
  // if send complete, reset offset to 0
  if (fd_node->offset >= fd_node->cache_node->buffer_size) {
    fd_node->status = IDLE;
    fd_node->offset = 0;
  }
  return numbytes;
}
// --- END SOCKET UTIL ---

// --- BEGIN UNIT TEST FUNCTIONS ---
void cache_init_test() {
  printf("\ncache init test\n");
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  printf("max slot: %d\n", cache_queue->max_slot);
  cache_node_t *cache_node = cache_queue->front;
  int count = 0;
  while (cache_node != NULL) {
    count += 1;
    print_cache_node(cache_node);
    cache_node = cache_node->next;
  }
  free_cache_queue(&cache_queue);
  printf("is cache queue null?%d\n", cache_queue == NULL);
  // printf("is cache queue front node null?%d\n", cache_queue->front == NULL);
}
void cache_enqueue_test() {
  printf("\ncache enqueue test\n");
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  cache_node_t *new_node = new_cache_node(NULL, NULL, 100);
  print_cache_node(new_node);
  char document[] = "hello world";
  memcpy(new_node->buffer, document, strlen(document));
  cache_enqueue(cache_queue, new_node);
  printf("new node buffer: %s\n", cache_queue->front->buffer);
  print_cache_queue(cache_queue);
  return;
}

void cache_eviction_test() {
  printf("\ncache eviction test\n");
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  cache_node_t *new_node;
  char document[] = "hello world";

  // hold address to front node after eviction
  new_node = new_cache_node(NULL, NULL, 50);
  new_node->status = IN_USE;
  memcpy(new_node->buffer, document, strlen(document));
  cache_enqueue(cache_queue, new_node);
  cache_node_t *evicted_node = cache_queue->front;
  printf("address before eviction: %p\n", evicted_node);
  print_cache_node(evicted_node);

  for (int i = 0; i < 13; i++) {
    new_node = new_cache_node(NULL, NULL, 100 + i);
    memcpy(new_node->buffer, document, strlen(document));
    cache_enqueue(cache_queue, new_node);
    // print_cache_node(evicted_node);
  }

  print_cache_queue(cache_queue);
  printf("address after eviction: %p\n", evicted_node);
  print_cache_node(evicted_node);
  return;
}

void fd_list_test() {
  printf("\nfd list test\n");
  fd_list_t *fd_list = new_fd_list(PROXY_MAX_CLIENT);
  print_fd_list(fd_list);
  fd_node_t *new_node = new_fd_node(NULL, NULL, 1, LISTEN, READING, NULL);
  fd_list_append(fd_list, new_node);
  print_fd_node(new_node);
  print_fd_list(fd_list);
  printf("max fd: %d\n", get_max_fd(fd_list));
  return;
}

http_err_t client_read_handler(fd_list_t *fd_list, fd_node_t *fd_node,
                               cache_queue_t *cache_queue) {
  fd_node_t *server_node, *tmp_node;
  cache_node_t *cache_hit_node;
  int server_fd;
  int numbytes = cache_recv(fd_node);
  char *buffer = fd_node->cache_node->buffer;

  if (fd_node->status == IDLE) {              // read complete, start parsing
    if (strstr(buffer, "\r\n\r\n") != NULL) { // contains \r\n\r\n
      parse_request(buffer, fd_node->cache_node->http_info);
      // print_fd_list(fd_list);
    }

    if (server_lookup_connect(fd_node->cache_node->http_info->host, "80",
                              &server_fd) != HTTP_OK) {
      // TODO: handle connection error
      tmp_node = fd_node->next;
      close(fd_node->fd);
      fd_list_remove(fd_node);
      free_fd_node(&fd_node);
      printf("connection failed.  fd cleanup\n");
    } else {
      server_node = new_fd_node(NULL, NULL, server_fd, SERVER, WRITING,
                                fd_node->cache_node);
      // bind server_node with client_node
      server_node->proxied = fd_node;
      fd_node->proxied = server_node;
      // serve after the next select call
      fd_list_append(fd_list, server_node);
      cache_hit_node =
          is_cache_hit(cache_queue, fd_node->cache_node->http_info);
      if (cache_hit_node != NULL) {
        printf("CACHE HIT!\n");
        // connect cache hit node to server
        fd_node->proxied->cache_node_backup = cache_hit_node;
        server_node->flag = 1; // conditional get on next loop
      } else {
        // connect and send normal get
        server_node->flag = 2; // normal get
        printf("CACHE MISS!\n");
      }
    }
  } else { // partially read request
    printf("client request received.\n");
    ;
  }
  // client read complete
  return HTTP_OK;
}

http_err_t listen_fd_handler(fd_list_t *fd_list, fd_node_t *fd_node) {
  int client_fd;
  fd_node_t *client_node;
  if (accept_client(fd_node->fd, &client_fd) != HTTP_OK) {
    return HTTP_FAIL;
  }
  client_node = new_fd_node(NULL, NULL, client_fd, CLIENT, READING,
                            new_cache_node(NULL, NULL, INITIAL_BUFFER));
  fd_list_append(fd_list, client_node);
  printf("client accepted.\n");
  return HTTP_OK;
}

http_err_t client_write_handler(fd_list_t *fd_list, fd_node_t *fd_node) {
  // send http response to client
  int numbytes = cache_send(fd_node);
  if (fd_node->status == IDLE) { // response sent complete
    printf("%ld/%ld Bytes to client\n", fd_node->cache_node->buffer_size,
           fd_node->cache_node->buffer_size);
    close(fd_node->fd);
    fd_list_remove(fd_node);
    free_fd_node(&fd_node);
  } else {
    // printf("%ld/%ld Bytes to client\n", fd_node->offset,
    //        fd_node->cache_node->buffer_size);
  }
  return HTTP_OK;
}

http_err_t server_read_handler(fd_list_t *fd_list, fd_node_t *fd_node,
                               cache_queue_t *cache_queue) {
  // read http response from server
  int numbytes = cache_recv(fd_node);
  if (numbytes != 0) {
    // printf("%ld/%s bytes from server\n", fd_node->offset,
    //        fd_node->cache_node->http_info->content_length);
  } else {
    printf("%s/%s bytes from server\n",
           fd_node->cache_node->http_info->content_length,
           fd_node->cache_node->http_info->content_length);
  }

  if (fd_node->status == IDLE) {        // received full response
    fd_node->proxied->status = WRITING; // start writing to client
    // add cache node to LRU cache list
    if (is_cache_hit(cache_queue, fd_node->cache_node->http_info) == NULL) {
      // if no cache hit
      cache_enqueue(cache_queue, fd_node->cache_node);
    }
    print_cache_queue(cache_queue);
    fd_list_remove(fd_node); // remove from select list
    free_fd_node(&fd_node);
  }
  return HTTP_OK;
}

http_err_t server_write_handler(fd_list_t *fd_list, fd_node_t *fd_node) {
  // write http request to server
  printf("writing to server\n");
  cache_send(fd_node);
  if (fd_node->status == IDLE) { // send complete
    fd_node->status = READING;   // waiting for server response
  }
  return HTTP_OK;
}
// --- END UNIT TEST FUNCTIONS ---
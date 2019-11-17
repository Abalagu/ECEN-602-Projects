#include "headers.h"
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
// --- END UNIT TEST FUNCTIONS ---

http_err_t main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: ./proxy <ip to bind> <port to bind>\n");
    return HTTP_FAIL;
  }
  // cache_init_test();
  // cache_enqueue_test();
  // cache_eviction_test();
  // fd_list_test();
  // return HTTP_OK;

  char *proxy_ip = argv[1], *proxy_port = argv[2];
  int listen_fd;
  int retval;
  int client_fd, server_fd;
  fd_node_t *client_node, *server_node;
  fd_set read_fds, write_fds;

  fd_list_t *fd_list = new_fd_list(PROXY_MAX_CLIENT);
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  if (server_init(proxy_port, &listen_fd) != HTTP_OK) {
    return HTTP_FAIL;
  };
  fd_node_t *fd_node =
      new_fd_node(NULL, NULL, listen_fd, LISTEN, READING, NULL);

  fd_list_append(fd_list, fd_node);
  while (1) {
    retval = fd_select(fd_list, &read_fds, &write_fds);
    if (retval == 0) {
      printf("nothing happens...\n");
      continue;
    } else if (retval == -1) {
      perror("select()");
      return HTTP_FAIL;
    }

    printf("something happens!\n");
    fd_node = fd_list->front;
    while (fd_node != NULL) {
      if (fd_node->type == DUMMY) {
        ;
      }
      if (fd_node->type == LISTEN && FD_ISSET(fd_node->fd, &read_fds)) {
        if (accept_client(fd_node->fd, &client_fd) != HTTP_OK) {
          return HTTP_FAIL;
        }
        client_node = new_fd_node(NULL, NULL, client_fd, CLIENT, READING,
                                  new_cache_node(NULL, NULL, INITIAL_BUFFER));
        fd_list_append(fd_list, client_node);
        // print_fd_list(fd_list);
        printf("client accepted.\n");
      } // client accepted

      if (fd_node->type == CLIENT) {
        // read http request from client
        if (fd_node->status == READING && FD_ISSET(fd_node->fd, &read_fds)) {
          cache_recv(fd_node);
          if (fd_node->status == IDLE) { // read complete, start parsing
            printf("%s\n", fd_node->cache_node->buffer);
            // TODO: add parsing from http request
            if (server_lookup_connect("www.wikipedia.org", "80", &server_fd) !=
                HTTP_OK) {
              // TODO: handle connection error
              fd_list_remove(fd_node);
              printf("connection failed.  fd cleanup\n");
            } else {
              server_node = new_fd_node(NULL, NULL, server_fd, SERVER, WRITING,
                                        fd_node->cache_node);
              // serve after the next select call
              server_node->proxied = fd_node;
              fd_node->proxied = server_node;
              fd_list_append(fd_list, server_node);
            };
          }
        } else if (fd_node->status == WRITING &&
                   FD_ISSET(fd_node->fd,
                            &write_fds)) { // send http response to client
          printf("CLIENT WRITE NOT IMPLEMENTED\n");
        }
      }

      if (fd_node->type == SERVER) {
        if (fd_node->status == READING && FD_ISSET(fd_node->fd, &read_fds)) {
          // read http response from server
          printf("SERVER READ NOT IMPLEMENTED\n");
        } else if (fd_node->status == WRITING &&
                   FD_ISSET(fd_node->fd, &write_fds)) {
          // write http request to server
          printf("writing to server\n");
          cache_send(fd_node);
          if (fd_node->status == IDLE) { // send complete
            fd_node->status = READING;   // waiting for server response
          }
        }
      }
      fd_node = fd_node->next;
    } // end node traversal
    printf("  end of fd node traversal\n");
  } // end service loop
  close(listen_fd);
  return HTTP_OK;
}

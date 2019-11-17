#include "headers.h"


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

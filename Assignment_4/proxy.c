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
  fd_node_t *client_node, *server_node, *tmp_node;
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
    // print_fd_list(fd_list);
    if (retval == 0) {
      printf(".");
      fflush(stdout);
      continue;
    } else if (retval == -1) {
      perror("select()");
      return HTTP_FAIL;
    }
    // normal service
    fd_node = fd_list->front;
    while (fd_node != NULL) {
      if (fd_node->type == DUMMY) {
        fd_node = fd_node->next;
        continue;
      } // skip dummy node

      // accept new connection
      if (fd_node->type == LISTEN) {
        if (FD_ISSET(fd_node->fd, &read_fds)) {
          tmp_node = fd_node->next;
          listen_fd_handler(fd_list, fd_node);
          print_fd_list(fd_list);
          fd_node = tmp_node;
        } else { // no incoming connection
          fd_node = fd_node->next;
        }
        continue;
      }

      // read http request from client
      if (fd_node->type == CLIENT) {
        if (fd_node->status == READING && FD_ISSET(fd_node->fd, &read_fds)) {
          tmp_node = fd_node->next;
          client_read_handler(fd_list, fd_node, cache_queue);
          fd_node = tmp_node;
        }
        // send http response
        else if (fd_node->status == WRITING &&
                 FD_ISSET(fd_node->fd, &write_fds)) {
          tmp_node = fd_node->next;
          client_write_handler(fd_list, fd_node);
        } else { // fd not set
          fd_node = fd_node->next;
        }
        continue;
      }

      if (fd_node->type == SERVER) {
        if (fd_node->status == READING && FD_ISSET(fd_node->fd, &read_fds)) {
          tmp_node = fd_node->next;
          server_read_handler(fd_list, fd_node, cache_queue);
          fd_node = tmp_node;
        } // http response read complete
        else if (fd_node->status == WRITING &&
                 FD_ISSET(fd_node->fd, &write_fds)) {
          tmp_node = fd_node->next;
          server_write_handler(fd_list, fd_node);
          fd_node = tmp_node;
        } else {
          fd_node = fd_node->next;
        } // http request send complete
        continue;
      } else {
        // nothing matched, current fd not set, go to next node
        fd_node = fd_node->next;
        continue;
      }
    } // end node traversal

    // printf("  end of fd node traversal\n");
  } // end service loop

  close(listen_fd);
  return HTTP_OK;
}

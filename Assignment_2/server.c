#include "server.h"

#define MAXDATASIZE 1500

int main(int argc, char *argv[]) {
  int sockfd, new_fd;
  int numbytes;
  int msg_type;
  sbcp_msg_t msg_send, *msg_recv;

  if (argc != 4) {  // check for correct usage
    fprintf(stderr, "usage: ./server server_ip server_port max_clients\n");
    exit(1);
  }
  char *server_ip = argv[1], *server_port = argv[2];
  int max_clients = atoi(argv[3]);

  // create server node
  sockfd = server_init(server_port);
  socket_fd_t *listen_fd = malloc(sizeof(socket_fd_t));
  listen_fd->fd = sockfd;
  strcpy(listen_fd->username, "server");
  listen_fd->next = NULL;

  // add select in server on sock_fd and stdin
  fd_set readfds;

  while (1) {
    fd_select(&readfds, *listen_fd);

    if (!FD_IS_ANY_SET(&readfds)) {  // timer expires
      // printf("expires.\n");
      print_nodes(listen_fd);
      continue;
    }


    msg_router(listen_fd, readfds);
    
    // printf("node traversal ends.\n");
  }  // while loop

  return 0;
}

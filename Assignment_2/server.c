#include "server.h"

#define MAXDATASIZE 1500

int main(int argc, char *argv[]) {
  int sockfd, new_fd;
  socklen_t addr_size;
  int numbytes;
  char buf[MAXDATASIZE];
  int msg_type;
  sbcp_msg_t msg_send, *msg_recv;

  if (argc != 4) {
    // check for correct usage
    fprintf(stderr, "usage: ./server server_ip server_port max_clients\n");
    exit(1);
  }
  char *server_ip = argv[1], *server_port = argv[2];
  int max_clients = atoi(argv[3]);

  sockfd = server_init(server_port);
  new_fd = connect_client(sockfd);

  // fill in test usernames
  char usernames[10][16] = {0};
  strcpy(usernames[0], "Akhilesh");
  strcpy(usernames[1], "Luming");
  strcpy(usernames[2], "P.Cantrell");

  // struct exchange test: server
  numbytes = server_read(new_fd, buf);
  msg_recv = (sbcp_msg_t *)buf;
  parse_msg_join(*msg_recv);

  msg_send = make_msg_ack(1, usernames);
  char reason[] = "same username";
  // *msg_send = make_msg_nak(reason, sizeof(reason));
  // memcpy(buf, &msg_nak, sizeof(msg_nak)); //SEND NAK TEST
  memcpy(buf, &msg_send, sizeof(sbcp_msg_t));  // SEND ACK TEST
  printf("sent ACK\n");

  numbytes = server_write(new_fd, buf);
  // add select in server on sock_fd and stdin
  struct timeval tv;
  fd_set readfds;

  while (1) {
    FD_ZERO(&readfds);
    // FD_SET(STDIN, &readfds);
    FD_SET(new_fd, &readfds);
    FD_SET(sockfd, &readfds);

    tv.tv_sec = 1;
    tv.tv_usec = 500000;
    if (new_fd > sockfd) {
      select(new_fd + 1, &readfds, NULL, NULL, &tv);
    } else {
      select(sockfd + 1, &readfds, NULL, NULL, &tv);
    }

    if (!FD_IS_ANY_SET(&readfds)) {  // timer expires
      printf("expires.\n");
      continue;
    }
    if (FD_ISSET(sockfd, &readfds)) {
      new_fd = connect_client(sockfd);
      printf("new client connects.");
    }

    if (FD_ISSET(new_fd, &readfds)) {  // a client sends msg
      printf("select on new_fd\n");
      numbytes = server_read(new_fd, buf);
      if (numbytes == 0) {
        printf("FIN received.\n");
        return 0;  // temporary handle of disconnection
      }

      msg_recv = (sbcp_msg_t *)buf;
      msg_type = get_msg_type(*msg_recv);
      if (msg_type == SEND) {
        parse_msg_send(*msg_recv);
        msg_send = make_msg_fwd(msg_recv->sbcp_attributes[0].payload,
                                sizeof(msg_recv->sbcp_attributes[0].payload),
                                "luming", 7);
        memcpy(buf, &msg_send, sizeof(sbcp_msg_t));
        server_write(new_fd, buf);
      }
    }  // if new_fd is set
  }

  return 0;
}

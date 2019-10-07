#include "client.h"

int main(int argc, char *argv[]) {
  // struct exchange test: client
  char buf[MAXDATASIZE];
  char recv_buf[MAXDATASIZE];
  char send_buf[MAX_MSG_LEN];
  sbcp_msg_t msg_send;
  int numbytes;

  if (argc != 4) {
    printf("usage: ./client username server_ip server_port\n");
    return 0;
  }

  printf("\n");
  char *my_name = argv[1];
  char message[] = "hello world!";

  // make simple connection to server
  int sock_fd;
  char *host = argv[2], *server_port = argv[3];
  sock_fd = server_lookup_connect(host, server_port);
  if (sock_fd < 0) {
    printf("connection error.\n");
    return 1;
  }
  printf("Connected to server. Joining...\n");

  int msg_type;
  // SEND JOIN TO SERVER
  sbcp_msg_t msg_join = make_msg_join(my_name, sizeof(my_name));
  memcpy(buf, &msg_join, sizeof(msg_join));
  writen(sock_fd, buf, sizeof(msg_join));
  readline(sock_fd, recv_buf);
  sbcp_msg_t *msg = (sbcp_msg_t *)recv_buf;
  msg_type = get_msg_type(*msg);
  // only deal with ACK and NAK, as not formally joined the chat yet
  if (msg_type == ACK) {
    if (parse_msg_ack(*msg, my_name) != 0) {
      return 0;  // message parse error.
    };
  } else if (msg_type == NAK) {
    parse_msg_nak(*msg);
    return 0;  // retry with different username
  } else {
    printf("message type error: %d\n", msg_type);
    return 0;  // routine error
  }
  // add select in client read from stdin or socket fd
  struct timeval tv;
  fd_set readfds;

  // start rx tx message with server
  while (1) {
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(sock_fd, &readfds);
    tv.tv_sec = 1;  // total of 1.5s waiting time
    tv.tv_usec = 500000;
    select(sock_fd + 1, &readfds, NULL, NULL, &tv);

    if (!FD_IS_ANY_SET(&readfds)) {
      // printf("time expires\n");
      continue;
    }
    if (FD_ISSET(STDIN, &readfds)) {
      fgets(send_buf, MAX_MSG_LEN - 1, stdin);
      // from SO, use strcspn to remove \n from stdin read
      send_buf[strcspn(send_buf, "\n")] = 0;
      msg_send = make_msg_send(send_buf, strlen(send_buf) + 1);
      memcpy(buf, &msg_send, sizeof(sbcp_msg_t));
      writen(sock_fd, buf, sizeof(sbcp_msg_t));
    }

    if (FD_ISSET(sock_fd, &readfds)) {
      numbytes = readline(sock_fd, recv_buf);
      if (numbytes == 0) {
        printf("server disconnect.\n");
        return 0;
      }
      msg = (sbcp_msg_t *)recv_buf;
      msg_type = get_msg_type(*msg);
      if (msg_type == FWD) {
        parse_msg_fwd(*msg);
      } else if (msg_type == OFFLINE) {
        parse_msg_offline(*msg);
      } else if(msg_type == ONLINE){
        parse_msg_online(*msg);
      }
    }
  }
  return 0;
}

#include "client.h"

int main(int argc, char *argv[]) {
  // struct exchange test: client
  char buf[MAXDATASIZE];
  char recv_buf[MAXDATASIZE];
  char send_buf[MAX_MSG_LEN];
  sbcp_msg_t msg_send;

  printf("\n");
  char username[] = "luming";
  char message[] = "hello world!";

  // make simple connection to server
  int sock_fd;
  char *host = "localhost", *server_port = "12345";
  sock_fd = server_lookup_connect(host, server_port);
  if (sock_fd < 0) {
    printf("connection error.\n");
    return 1;
  }
  printf("connected\n");

  int msg_type;
  // SEND JOIN TO SERVER
  sbcp_msg_t msg_join = make_msg_join(username, sizeof(username));
  memcpy(buf, &msg_join, sizeof(msg_join));
  writen(sock_fd, buf, sizeof(msg_join));
  readline(sock_fd, recv_buf);
  sbcp_msg_t *msg = (sbcp_msg_t *)recv_buf;
  msg_type = get_msg_type(*msg);
  if (msg_type == ACK) {
    printf("join success\n");
    parse_msg_ack(*msg);
  } else if (msg_type == NAK) {
    printf("NAK received.\n");
    parse_msg_nak(*msg);
    return 0;  // retry with different username
  } else {
    printf("message type error\n");
    return 0;  // routine error
  }
  // start rx tx message with server

  while (1) {
    printf("send:\n");
    fgets(send_buf, MAX_MSG_LEN - 1, stdin);
    msg_send = make_msg_send(send_buf, strlen(send_buf)+1);
    memcpy(buf, &msg_send, sizeof(sbcp_msg_t));
    writen(sock_fd, buf, sizeof(sbcp_msg_t));

  }
  return 0;
}

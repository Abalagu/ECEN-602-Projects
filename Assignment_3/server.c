#include "config.h"
#include "lib.h"

tftp_err_t main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: ./server port\n");
    return 1;
  }

  char *port;
  port = argv[1];

  char buf_recv[MAXBUFLEN];
  int listen_fd;
  size_t numbytes;
  
  opcode_t opcode;
  // struct sockaddr_storage their_addr;
  struct sockaddr client_addr;

  /* initialize the server */
  init(port, &listen_fd);

  tftp_recvfrom(listen_fd, buf_recv, &numbytes, &client_addr);
  if (!fork()) {
    close(listen_fd); // close listening socket in child process

    parse_header(buf_recv, numbytes, &opcode);
    if (opcode == RRQ) {
      rrq_handler(buf_recv, numbytes, client_addr);
    }

    printf("child returns.\n");
  } else {
    wait(NULL);
  }

  close(listen_fd);
  return TFTP_OK;
}
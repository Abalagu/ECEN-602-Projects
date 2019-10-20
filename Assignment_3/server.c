#include "config.h"
#include "lib.h"

tftp_err_t main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: ./server port\n");
    return 1;
  }
  // TODO: add port param check
  char *port;
  port = argv[1];

  char buf_recv[MAXBUFLEN];
  int listen_fd;
  size_t numbytes;
  opcode_t opcode;
  struct sockaddr client_addr;

  /* initialize the server */
  if (init(port, &listen_fd) == TFTP_FAIL) {
    printf("INIT FAILED.\n");
    return TFTP_FAIL;
  }

  while (1) {
    tftp_recvfrom(listen_fd, buf_recv, &numbytes, &client_addr);
    if (!fork()) {
      close(listen_fd); // close listening socket in child process

      parse_header(buf_recv, numbytes, &opcode);
      if (opcode == RRQ) {
        if (rrq_handler(buf_recv, numbytes, client_addr) == TFTP_OK) {
          printf("EXIT ON OK.\n");
        } else {
          printf("EXIT ON ERROR.\n");
        }
      }
      if (opcode == WRQ) {
        // TODO: add WRQ handling
        printf("WRQ not handled.\n");
      }
      exit(0);
    } else {
      wait(NULL);
      printf("parent wait end\n\n");
    }
  }

  close(listen_fd);
  return TFTP_OK;
}
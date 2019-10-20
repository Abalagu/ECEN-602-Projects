#include "config.h"
#include "lib.h"

tftp_err_t main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: ./server port\n");
    return 1;
  }

  char *port;
  port = argv[1];

  char buf[MAXBUFLEN], filename[MAX_FILE_NAME] = {0};
  int listen_fd, numbytes;
  tftp_mode_t mode;
  opcode_t opcode;
  struct sockaddr_storage their_addr;

  /* initialize the server */
  init(port, &listen_fd);

  tftp_recvfrom(listen_fd, buf, &numbytes, &their_addr);
  if (!fork()) {
    close(listen_fd); // close listening socket in child process

    parse_header(buf, numbytes, &opcode);
    if (opcode == RRQ) {
      parse_rrq(buf, numbytes, filename, &mode);

      rrq_handler(filename, mode, (const struct sockaddr *)&their_addr);
    }

    printf("child returns.\n");
  } else {
    wait(NULL);
  }
  while (1) { // loop to maintain parent process
  }

  close(listen_fd);
  return TFTP_OK;
}
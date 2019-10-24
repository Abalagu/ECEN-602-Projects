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
      // child process
      close(listen_fd); // close listening socket in child process

    	if (DEBUG) {
    		printf("[Packet size] %ld\n", numbytes);
    		printf("[Packet content] : \n" );
    		for (int i = 0; i < numbytes; i++) {
    			printf(" [%d]:", i);
    			printf(" %d %c\n",buf_recv[i], buf_recv[i]);
    		} 
    	}

      parse_header(buf_recv, &opcode);
      if (opcode == RRQ) {
        if (rrq_handler(buf_recv, numbytes, client_addr) == TFTP_OK) {
          printf("EXIT ON OK.\n");
        } else {
          printf("EXIT ON ERROR.\n");
        }
      }
      if (opcode == WRQ) {
        // TODO: add WRQ handling
        if (wrq_handler(buf_recv, numbytes, client_addr) == TFTP_OK) {
          printf("TFTP_OK\n");
        } else {
          printf("TFTP_FAIL\n");
        }
        // printf("WRQ not handled.\n");
      }
      exit(0);
    } else {
      // parent process
      continue;
    }
  }

  close(listen_fd);
  return TFTP_OK;
}
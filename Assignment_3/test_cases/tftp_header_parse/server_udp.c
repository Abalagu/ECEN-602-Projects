
#include "server_lib.h"

int main(void) {
  int numbytes;
  struct sockaddr_storage their_addr;
  char buf[MAXBUFLEN];
  socklen_t addr_len;
  char s[INET6_ADDRSTRLEN];
  int sockfd;

  sockfd = launch_udp_server();
  printf("config RRQ: %d\n", RRQ);

  addr_len = sizeof their_addr;
  while (1) {
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                             (struct sockaddr *)&their_addr, &addr_len)) ==
        -1) {
      perror("recvfrom");
      exit(1);
    }

    printf("listener: got packet from %s\n",
           inet_ntop(their_addr.ss_family,
                     get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    print_hex(buf, numbytes);
    opcode_t opcode = get_type(buf, numbytes);
    printf("opcode: %d\n", opcode);
  }
  close(sockfd);

  return 0;
}
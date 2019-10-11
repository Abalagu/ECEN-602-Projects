#include "server_lib.h"

// given rrq packet, return filename and mode
tftp_error_t parse_rrq(char *packet, size_t len, char *filename,
                       tftp_mode_t *mode) {
  char octet[] = "OCTET";
  char netascii[] = "NETASCII";

  // count instance of 0, expect 3, otherwise it's a corrupt RRQ message
  int count = 0;
  int position[3] = {-1};
  // record 0 position in array for future memcpy
  for (int i = 0; i < len; i++) {
    if (packet[i] == 0) {
      position[count] = i;
      count += 1;
    }
    if (count > 3) {  // corrupt RRQ message
      return TFTP_ERR;
    }
  }

  // extract filename from RRQ packet. What if filename contains path?
  int name_len = position[1] - position[0] - 1;
  memcpy(filename, packet + 2, name_len);  // offset due to opcode

  // extract mode from RRQ packet
  if (TFTP_OK == strncasecmp(packet + position[1] + 1, octet, strlen(octet))) {
    *mode = OCTET;
  } else if (TFTP_OK == strncasecmp(packet + position[1] + 1, netascii,
                                    strlen(netascii))) {
    *mode = NETASCII;
  } else {
    *mode = -1;
    return TFTP_ERR;  // unknown mode
  }

  return TFTP_OK;
}

// get type from UDP packet. return -1 for unknown packet type.
opcode_t get_type(char *packet, size_t packet_len) {
  uint8_t opcode;
  if (packet[0] != 0) {
    return TFTP_ERR;
  }
  memcpy(&opcode, packet + 1, 1);  // copy second byte to opcode
  switch (opcode) {
    case RRQ:
      return RRQ;
    case WRQ:
      return WRQ;
    case DATA:
      return DATA;
    case ACK:
      return ACK;
    case ERROR:
      return ERROR;
    default:
      return TFTP_ERR;
  }
}

void print_hex(void *array, size_t len) {
  char *parray = array;
  for (int i = 0; i < len; i++) {
    printf(" %02x", parray[i]);
  }
  printf("\n");
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int launch_udp_server() {
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int sockfd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;  // use my IP

  if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);
  printf("listener: waiting to recvfrom...\n");

  return sockfd;
}

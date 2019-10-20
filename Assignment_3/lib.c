#include "lib.h"
// ------- START OF INNER UTIL FUNCTIONS ------------
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// given sockfd, return local port
tftp_err_t get_sock_port(int sockfd, int *local_port) {
  struct sockaddr_in sin;
  int addrlen = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &addrlen) == 0 &&
      sin.sin_family == AF_INET && addrlen == sizeof(sin)) {
    *local_port = ntohs(sin.sin_port);
    return TFTP_OK;
  } else {
    printf("find port error.\n");
    return TFTP_FAIL;
  }
}

// print buffer of given length in hexadecimal form, prefix with 0
void print_hex(void *array, size_t len) {
  char *parray = array;
  for (int i = 0; i < len; i++) {
    printf(" %02x", parray[i]);
  }
  printf("\n");
}
// copy struct to buffer, handle endian issue
int data_to_buffer(char *buf_send, tftp_data_t data_packet) {
  off_t offset = 0;
  uint16_t big_endian = (data_packet.opcode >> 8) | (data_packet.opcode << 8);
  memcpy(buf_send, &(big_endian), sizeof(big_endian));
  offset += sizeof(data_packet.opcode);

  big_endian = (data_packet.block_num >> 8) | (data_packet.block_num << 8);
  memcpy(buf_send + offset, &(big_endian), sizeof(big_endian));
  offset += sizeof(data_packet.block_num);

  memcpy(buf_send + offset, data_packet.payload, sizeof(data_packet.payload));
  return 0;
}

int read_block(char *filename, char *buf) {

  FILE *fp;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("file: %s. open failed.\n", filename);
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  off_t size = ftell(fp); // long int, get file size
  printf("filename: %s, size: %ld\n", filename, size);

  fseek(fp, 0, SEEK_SET);
  size_t numbytes = fread(buf, 1, MAXBUFLEN, fp);

  return numbytes;
}

void get_peer_info(int sockfd, struct sockaddr_storage addr) {
  // assume s is a connected socket

  socklen_t len = sizeof addr;
  char ipstr[INET6_ADDRSTRLEN];
  int port;

  getpeername(sockfd, (struct sockaddr *)&addr, &len);

  // deal with both IPv4 and IPv6:
  if (addr.ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
  } else { // AF_INET6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
    port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
  }

  printf("Peer IP address: %s\n", ipstr);
  printf("Peer port      : %d\n", port);
}
// ------- END OF INNER UTIL FUNCTIONS ------------

tftp_err_t init(char *port, int *sockfd) {
  int rv, numbytes;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return TFTP_FAIL;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("listener: socket");
      continue;
    }
    if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }
  if (p == NULL) {
    return TFTP_FAIL;
  }

  freeaddrinfo(servinfo);
  int local_port;
  get_sock_port(*sockfd, &local_port);
  printf("TFTP server port: %d\n", local_port);
  return TFTP_OK;
}

tftp_err_t tftp_recvfrom(int listen_fd, char *buf, int *numbytes,
                         struct sockaddr_storage *their_addr) {
  char s[INET6_ADDRSTRLEN] = {0};
  socklen_t addr_len = sizeof their_addr;

  printf("server: waiting to recvfrom \n");
  if ((*numbytes = recvfrom(listen_fd, buf, MAXBUFLEN - 1, 0,
                            (struct sockaddr *)their_addr, &addr_len)) == -1) {
    perror("recvform");
    return TFTP_FAIL;
  }
  get_peer_info(listen_fd, *their_addr);

  return TFTP_OK;
}

tftp_err_t parse_header(char *buf, size_t numbytes, opcode_t *opcode) {
  print_hex(buf, 4);
  tftp_header_t *tftp_header = (tftp_header_t *)buf;
  /* extract opcode */
  *opcode = (tftp_header->opcode) >> 8;
  return TFTP_OK;
}

tftp_err_t parse_rrq(char *buf, size_t len_buf, char *filename,
                     tftp_mode_t *mode) {
  /* trailing buffer now has filename and the mode. According to RFC, the file
        name is followed by a 0. Locate that 0 */
  tftp_header_t *tftp_header = (tftp_header_t *)buf;
  char _mode[9]; // netascii, octet or mail, should never exceed 8 bytes

  char *pos_ptr = strchr(tftp_header->trail_buf, 0);
  /* in case pos reaches the end of the file, it's a problem */
  int pos = (pos_ptr == NULL ? -1 : pos_ptr - tftp_header->trail_buf);

  if (pos < 0) {
    perror("filename not valid\n");
    // TODO:: send an error.
    return TFTP_FAIL;
  }

  /* extract filename from the trail buffer */
  memcpy(filename, tftp_header->trail_buf, pos);

  /* rest of the buffer has mode */
  memcpy(_mode, &(tftp_header->trail_buf[pos + 1]), 8 * sizeof(char));

  printf("request file: %s, mode: %s\n", filename, _mode);
  return TFTP_OK;
}

void rrq_handler(char *filename, tftp_mode_t mode,
                 const struct sockaddr *client_addr) {
  char buf_send[516] = {0};
  bool next_block = 1;
  int numbytes = 0;
  int sockfd;
  init("", &sockfd);
  if (access(filename, F_OK) == -1) { // file doesn't exist
    printf("file %s doesn't exist.\n", filename);
    return; // early return
  }
  // create new socket with ephemeral port

  tftp_data_t data_packet = {0};
  data_packet.opcode = DATA;
  data_packet.block_num = 1;
  numbytes = read_block(filename, data_packet.payload);
  printf("numbytes read: %d\n", numbytes);

  data_to_buffer(buf_send, data_packet);
  print_hex(buf_send, 4);
  sendto(sockfd, buf_send, sizeof(buf_send), 0, client_addr,
         sizeof(*client_addr));
}

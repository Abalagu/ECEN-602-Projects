#include "lib.h"

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int get_sock_port(int sockfd) {
  struct sockaddr_in sin;
  int addrlen = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &addrlen) == 0 &&
      sin.sin_family == AF_INET && addrlen == sizeof(sin)) {
    int local_port = ntohs(sin.sin_port);
    return local_port;
  } else {
    printf("find port error.\n");
    return -1;
  }
}

int init(char *port) {
  int sockfd, rv, numbytes;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
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
    return 2;
  }

  freeaddrinfo(servinfo);
  printf("assigned port: %d\n", get_sock_port(sockfd));
  return sockfd;
}

void print_hex(void *array, size_t len) {
  char *parray = array;
  for (int i = 0; i < len; i++) {
    printf(" %02x", parray[i]);
  }
  printf("\n");
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
void rrq_handler(char *filename, const struct sockaddr *client_addr) {
  char buf_send[516] = {0};
  bool next_block = 1;
  int numbytes = 0;
  int sockfd = init("");
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

opcode_t parse_header(char buf[MAXBUFLEN], char filename[MAXBUFLEN],
                      char mode[8]) {
  /* parse the buffer and fill in filename and mode and return the opcode */

  tftp_header_t *tftp_header = (tftp_header_t *)buf;
  /* extract opcode */
  opcode_t opcode = (tftp_header->opcode) >> 8;

  if (DEBUG) {
    printf("[Opcode] %d\n", opcode);
  }

  /* trailing buffer now has filename and the mode. According to RFC, the file
          name is followed by a 0. Locate that 0 */
  char *pos_ptr = strchr(tftp_header->trail_buf, 0);
  /* in case pos reaches the end of the file, it's a problem */
  int pos = (pos_ptr == NULL ? -1 : pos_ptr - tftp_header->trail_buf);

  if (pos < 0) {
    perror("filename not valid\n");
    // TODO:: send an error.
  }

  /* extract filename from the trail buffer */
  memcpy(filename, tftp_header->trail_buf, pos);

  if (DEBUG) {
    printf("[Passed filename] ");
    for (int i = 0; i < pos; i++) {
      printf("%c", filename[i]);
    }
    printf("\n");
  }

  /* rest of the buffer has mode */
  memcpy(mode, &(tftp_header->trail_buf[pos + 1]), 8 * sizeof(char));

  if (DEBUG) {
    printf("[Mode] ");
    for (int i = 0; i < 8; i++) {
      printf("%c", mode[i]);
    }
    printf("\n");
  }

  return opcode;
}
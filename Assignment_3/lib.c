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


char nextchar = -1;

// given block_num, read file to buffer
int read_block(FILE **fp, uint16_t block_num, char *buf, char *mode) {
  if (!strcmp(mode, "netascii")) {
    size_t numbytes;

    // size_t numbytes = fread(buf, 1, MAXBUFLEN, *fp);
    char c;
    for (numbytes = 0; numbytes < MAXBUFLEN; numbytes++) {

      if (nextchar >= 0) {  
        *buf++ = nextchar;
        nextchar = -1;
        continue;
      }
      
      c = getc(*fp);

      if (c == EOF) {
        if (ferror(*fp)) {
          // read error
          printf("READ ERROR FROM GETC ON LOCAL FILE\n");
        }
        return numbytes;
      } else if (c == '\n') {
        c = '\r';
        nextchar = '\n';

      } else if (c == '\r') {
        nextchar  = '\0';

      } else 
        nextchar = -1;

      *buf++ = c;

    }

    return numbytes;
  } else if (!strcmp(mode, "octet")) {
    size_t numbytes = fread(buf, 1, MAXBUFLEN, *fp);
    // TODO: #block print disabled for performance issue
    // printf("#block: %d, size: %ld\n", block_num, numbytes);
    return numbytes;
  } else {
    return -1;
  }
}

// get connected peer (remote) IP address port number
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

  // if (DEBUG) {
  //   printf("Peer IP address: %s\n", ipstr);
  //   printf("Peer port      : %d\n", port);
  // }
}

// copy error packet to buffer, handle endian issue
tftp_err_t error_to_buffer(char *buf_send, tftp_error_packet_t error_packet) {
  off_t offset = 0;
  uint16_t big_endian = bswap_16(error_packet.opcode);
  memcpy(buf_send, &big_endian, sizeof(big_endian));
  offset += sizeof(error_packet.opcode);

  big_endian = bswap_16(error_packet.error_code);
  memcpy(buf_send + offset, &big_endian, sizeof(big_endian));
  offset += sizeof(error_packet.error_code);

  memcpy(buf_send + offset, error_packet.error_msg,
         sizeof(error_packet.error_msg));
  return TFTP_OK;
}

//  given error code and error message, write to buffer
// TODO: add other possible error code logic
tftp_err_t make_error_packet(error_code_t error_code, char *error_msg,
                             char *buf_send) {
  tftp_error_packet_t error_packet = {0};
  error_packet.opcode = ERROR;
  error_packet.error_code = error_code;
  strcpy(error_packet.error_msg, error_msg);
  error_to_buffer(buf_send, error_packet);

  return TFTP_OK;
}

// copy struct to buffer, handle endian issue
tftp_err_t data_to_buffer(char *buf_send, tftp_data_packet_t data_packet) {
  off_t offset = 0;
  uint16_t big_endian = bswap_16(data_packet.opcode);
  memcpy(buf_send, &big_endian, sizeof(big_endian));
  offset += sizeof(data_packet.opcode);

  big_endian = bswap_16(data_packet.block_num);
  memcpy(buf_send + offset, &big_endian, sizeof(big_endian));
  offset += sizeof(data_packet.block_num);

  memcpy(buf_send + offset, data_packet.payload, sizeof(data_packet.payload));
  return TFTP_OK;
}

char _nextchar = -1;

size_t write_data_to_file(FILE **fd, char *buf, size_t _numbytes, char *mode) {
  if (!strcmp(mode, "netascii")) {
    size_t numbytes;
    // numbytes = fwrite(buf, 1, _numbytes, *fd);

    char c;
    for (numbytes = 0; numbytes < _numbytes; numbytes++) {

      if (_nextchar >= 0) {  
        // *buf++ = nextchar;
        putc(_nextchar, *fd);
        _nextchar = -1;
        continue;
      }
      
      c = buf[numbytes];

      if (c == EOF) {
        return numbytes;
      } else if (c == '\r') {
        if (buf[numbytes+1] == '\0') {
          numbytes++;
        } else if (buf[numbytes+1] == '\n') {
          c = '\n';
          numbytes++;
        }
      } else 
        _nextchar = -1;

      // *buf++ = c;
      putc(c, *fd);

    }

    return numbytes;
  } else if (!strcmp(mode, "octet")) {
    size_t numbytes = fwrite(buf, 1, _numbytes, *fd);
    // printf("buf size written:%lu\n", num_bytes);
    return numbytes;
  } else {
    return -1;
  }
}

// read from fp into buf_send, numbytes read returned for EOF decision
tftp_err_t make_data_packet(FILE **fp, uint16_t block_num, char *buf_send,
                            size_t *numbytes, char *mode) {
  tftp_data_packet_t data_packet = {0};
  data_packet.opcode = DATA;
  data_packet.block_num = block_num;
  *numbytes = read_block(fp, block_num, data_packet.payload, mode); 

  if (*numbytes == -1) {
    // error in local file read
    return TFTP_FAIL;
  }

  data_to_buffer(buf_send, data_packet);

  return TFTP_OK;
}

// prepare to be sent on the network
tftp_err_t ack_to_buffer(char *buf_send, tftp_ack_packet_t ack_packet) {
  off_t offset = 0; 
  uint16_t big_endian = bswap_16(ack_packet.opcode);
  memcpy(buf_send, &big_endian, sizeof big_endian);
  offset += sizeof(ack_packet.opcode);

  big_endian = bswap_16(ack_packet.block_num);
  memcpy(buf_send + offset, &big_endian, sizeof big_endian);
  // printf("ack:\n");
  // print_hex(buf_send, sizeof(buf_send));

  return TFTP_OK;
}

// create an ack with the block num
tftp_err_t make_ack_packet(char *buf_send, uint16_t block_num) {
  tftp_ack_packet_t ack_packet = {0};
  ack_packet.opcode = ACK;
  ack_packet.block_num = block_num; 

  ack_to_buffer(buf_send, ack_packet);
  return TFTP_OK;
}


// opcode is verified in the packet router
// compare if ack to the same block_num counter
tftp_err_t parse_ack_packet(char *buf_recv, uint16_t block_num) {
  tftp_ack_packet_t *ack_packet = (tftp_ack_packet_t *)buf_recv;

  if (bswap_16(ack_packet->block_num) == block_num) {
    // TODO: ACK print disabled
    // printf("ACK on block: %d\n", bswap_16(ack_packet->block_num));
    return TFTP_OK;
  } else {
    printf("WRONG BLOCK NUM: %d. Expect: %d\n", bswap_16(ack_packet->block_num),
           block_num);
    return TFTP_FAIL;
  }
}


tftp_err_t parse_data_packet(char *buf, uint16_t *block_num, size_t *numbytes) {
  tftp_data_packet_t *data_packet = (tftp_data_packet_t *) buf;

  // ntoh order
  *block_num = bswap_16(data_packet->block_num);
  memcpy(buf, data_packet->payload, *numbytes);
  return TFTP_OK;
}


// given buffer and length, return filename and mode
tftp_err_t parse_trail_buf(char *buf, size_t len_buf, char *filename,
                     char *mode) {
  /* trailing buffer now has filename and the mode. According to RFC, the file
        name is followed by a 0. Locate that 0 */
  tftp_header_t *tftp_header = (tftp_header_t *)buf;

  char *pos_ptr = strchr(tftp_header->trail_buf, 0);
  /* in case pos reaches the end of the file, it's a problem */
  int pos = (pos_ptr == NULL ? -1 : pos_ptr - tftp_header->trail_buf);

  if (pos < 0) {
    perror("filename not valid\n");
    return TFTP_FAIL;
  }

  /* extract filename from the trail buffer */
  memcpy(filename, tftp_header->trail_buf, pos);

  /* rest of the buffer has mode */
  memcpy(mode, &(tftp_header->trail_buf[pos + 1]), 8 * sizeof(char));

  printf("request file: %s, mode: %s\n", filename, mode);
  return TFTP_OK;
}


// given file name and mode, open file, 
tftp_err_t open_file(FILE **fp, char *filename, char *mode) {
  *fp = fopen(filename, mode);
  if (*fp == NULL) {
    printf("file: %s. open failed.\n", filename);
    return TFTP_FAIL;
  } else {
    return TFTP_OK;
  } 
}

// ------- END OF INNER UTIL FUNCTIONS ------------

// launch DGRAM socket at given port. port = "" results in ephemeral port.
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

tftp_err_t tftp_recvfrom(int sockfd, char *buf, size_t *numbytes,
                         struct sockaddr *their_addr) {
  // char s[INET6_ADDRSTRLEN] = {0};
  socklen_t addr_len = sizeof their_addr;

  // accouting for maximum data receivable
  if ((*numbytes = recvfrom(sockfd, buf, DATA_PACKET_LEN, 0, their_addr,
                            &addr_len)) == -1) {
    perror("recvform");
    return TFTP_FAIL;
  }
  get_peer_info(sockfd, *(struct sockaddr_storage *)their_addr);

  return TFTP_OK;
}

tftp_err_t parse_header(char *buf, opcode_t *opcode) {
  tftp_header_t *tftp_header = (tftp_header_t *)buf;
  /* extract opcode */
  *opcode = bswap_16(tftp_header->opcode);
  return TFTP_OK;
}

// given RRQ buffer, its length, and remote address, enter handling routine
tftp_err_t rrq_handler(char *buf, size_t numbytes,
                       struct sockaddr client_addr) {
  char buf_send[516] = {0}, buf_recv[516] = {0}, error_msg[128] = {0},
       filename[MAX_FILE_NAME] = {0}, mode[8] = {0};

  // mode: netascii, octet or mail, should never exceed 8 bytes

  bool is_EOF = 0; // if is last data block, send 1 less byte
  // local block num counter, pass to file read, verify on ACK
  // use uint16_t for natural rollover when overflow
  uint16_t block_num = 1;
  int sockfd;
  // tftp_mode_t mode;
  opcode_t opcode;
  int timeout_counter = 0;
  FILE *fp;
  fd_set readfds;
  struct timeval tv;

  if (parse_trail_buf(buf, numbytes, filename, mode) != TFTP_OK) {
    printf("RRQ PARSE ERROR.\n");
    return TFTP_FAIL;
  }

  if (init("", &sockfd) != TFTP_OK) { // create with an ephemeral port
    printf("INIT FAIL.\n");
    return TFTP_FAIL;
  }

  if (access(filename, F_OK) == -1) { // file doesn't exist
    char copy_fn[10];
    strncpy(copy_fn, filename, 10);
    copy_fn[9] = '\0'; // trunc it to 10, else filename >> error_msg   
    sprintf(error_msg, "file '%s'... not found.", copy_fn);

    printf("%s\n", error_msg);
    make_error_packet(FILE_NOT_FOUND, error_msg, buf_send);
    sendto(sockfd, buf_send, sizeof(buf_send), 0, &client_addr,
           sizeof(client_addr));
    return TFTP_FAIL; // early return
  }

  if (open_file(&fp, filename, "r") != TFTP_OK) {
    printf("OPEN FILE FAIL!.\n");
    // TODO: reply with error packet.
    return TFTP_FAIL;
  };

  if (DEBUG) {
    fseek(fp, 0, SEEK_END);
    off_t size = ftell(fp); // long int, get file size
    printf("filename: %s, size: %ld\n", filename, size);
    fseek(fp, 0, SEEK_SET);
  }

  while (1) {
    make_data_packet(&fp, block_num, buf_send, &numbytes, mode);
    // if read less than 512 bytes of data, reached EOF
    is_EOF = (numbytes < MAXBUFLEN);

    sendto(sockfd, buf_send, 4 + numbytes, 0, &client_addr,
           sizeof(client_addr));
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    if (select(sockfd + 1, &readfds, NULL, NULL, &tv) == 0) {
      timeout_counter += 1;
      if (timeout_counter == MAX_RETRY) {
        // disconnect on 10 consecutive timeout
        close(sockfd);
        printf("remote disconnected. %d consecutive timeout.\n", MAX_RETRY);
        return TFTP_FAIL;
      }
      continue; // skip below routine, try to resend
    };

    // select(sockfd,)
    tftp_recvfrom(sockfd, buf_recv, &numbytes, &client_addr);
    parse_header(buf_recv, &opcode);

    if (opcode != ERROR && opcode != ACK) { // deal with trivial error case
      // take no action on other packets, make disconnect decision from timeout
      printf("UNKNOWN PACKET\n");
    }
    if (opcode == ERROR) {
      printf("ERROR PACKET\n");
      return TFTP_FAIL; // end of routine on FAIL
    }
    if (opcode == ACK) {
      if (parse_ack_packet(buf_recv, block_num) == TFTP_OK) {
        block_num += 1;
        if (is_EOF) {     // received final ACK
          close(sockfd);  // tftp sends get to well-known port each time.
          return TFTP_OK; // end of routine on success
        }
      } else { // wrong block number
        printf("WRONG #BLOCK.\n");
      }
    }
  } // end of while loop
} // end of rrq handler routine


// handle write request to the server
tftp_err_t wrq_handler(char *buf, size_t numbytes, struct sockaddr client_addr) {

	char buf_send[516] = {0}, buf_recv[516] = {0}, error_msg[128] = {0},
				filename[MAX_FILE_NAME] = {0}, mode[8] = {0};
        
  // mode: netascii, octet or mail, should never exceed 8 bytes

	uint16_t block_num = 0, last_block_ack = 0;
  int sockfd, timeout_counter = 0;
  opcode_t opcode;
  FILE *fd;
  fd_set readfds;
  struct timeval tv;
  int _EOF = 0;

  // parse the trailing buf and return filname and mode
  if (parse_trail_buf(buf, numbytes, filename, mode) != TFTP_OK) {
    printf("WRQ PARSE ERROR. \n");
    return TFTP_FAIL;
  }

  if (init("", &sockfd) != TFTP_OK) {
    printf("INIT FAIL. \n");
    return TFTP_FAIL;
  }

  if(access(filename, F_OK) != -1) {
    // file exists
    char copy_fn[10];
    strncpy(copy_fn, filename, 10);
    copy_fn[9] = '\0';
    sprintf(error_msg, "file '%s'... already exists.", copy_fn);

    printf("%s\n", error_msg);
    make_error_packet(FILE_EXISTS, error_msg, buf_send);
    // inform the client
    sendto(sockfd, buf_send, sizeof(buf_send), 0, &client_addr, sizeof(client_addr)); 
    return TFTP_FAIL;
  }

  // open file for write
  if (open_file(&fd, filename, "a") != TFTP_OK) {
    printf("FILE WRITE FAIL!\n");
    // TODO: reply with error packet.
    return TFTP_FAIL;
  }

  while(1) {
    if (make_ack_packet(buf_send, block_num) != TFTP_OK) {
      printf("server: ACK not sent. Connection failed.\n");
    } 

    if (sendto(sockfd, buf_send, 4, 0, &client_addr, sizeof client_addr) == -1) {
      printf("server: ACK not sent, Connection failed\n");
    } 

    // update block ack 
    last_block_ack = block_num;

    if (DEBUG)
      printf("last_block_ack:%d\n", last_block_ack);

    if (_EOF) {
      // safely return
      close(sockfd);
      return TFTP_OK;
    }

    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    if (select(sockfd + 1, &readfds, NULL, NULL, &tv) == 0) {
      timeout_counter += 1;
      if (timeout_counter == MAX_RETRY) {
        // disconnect on MAX_RETRY attempts
        close(sockfd);
        printf("remote disconnected. %d consecutive timeout.\n", MAX_RETRY);
        return TFTP_FAIL;
      } else {
        continue;
      }
    }

    // client will send the next data packet if recieved an ACK
    tftp_recvfrom(sockfd, buf_recv, &numbytes, &client_addr);

    if (DEBUG)
      printf("recvfrom numbytes: %lu\n", numbytes);
    // parse recieved packet
    parse_header(buf_recv, &opcode);

    // opcode should be either DATA or ERROR at this point
    if ((opcode != DATA) && (opcode != ERROR)) {
      printf("UNKNOWN PACKET\n");
      return TFTP_FAIL;
    }

    if (opcode == ERROR) {
      printf("ERROR PACKET\n");
      return TFTP_FAIL;
    }

    if (opcode == DATA) {
      numbytes -= 4;      // accouting for opcode and block number
      if (parse_data_packet(buf_recv, &block_num, &numbytes) == TFTP_OK) {

        if (DEBUG)
          printf("block Number:%d\n", block_num);

        if (block_num > last_block_ack) {
          write_data_to_file(&fd, buf_recv, numbytes, mode);
        } else {
          // Duplicate Frame Recieved, Send an ACK again
        }
        if (numbytes < MAXBUFLEN) {
          _EOF = 1;
        }
      }
    }
  } 
}

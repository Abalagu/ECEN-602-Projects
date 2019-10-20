#ifndef LIB_H_
#define LIB_H_

#include "config.h"

/* types of tftp packets */
typedef enum { RRQ = 1, WRQ = 2, DATA = 3, ACK = 4, ERROR = 5 } opcode_t;

// status code of lib function return value
typedef enum { TFTP_OK = 0, TFTP_FAIL = -1 } tftp_err_t;

// define enum for two file type
typedef enum { NETASCII = 1, OCTET = 2 } tftp_mode_t;

typedef struct {
  uint16_t opcode;
  char trail_buf[MAXBUFLEN];
} tftp_header_t;

typedef struct {
  uint16_t opcode;
  uint16_t block_num;
  char payload[512];
} tftp_data_t;

/* initialize server */
tftp_err_t init(char *port, int *sockfd);

// parent function. listen on well known port.
// return buf, numbytes, and remote address
tftp_err_t tftp_recvfrom(int listen_fd, char *buf, int *numbytes,
                         struct sockaddr_storage *their_addr);

// given buffer and numbytes, return opcode from its header
tftp_err_t parse_header(char *buf, size_t numbytes, opcode_t *opcode);

// given buffer and length, return filename and mode
tftp_err_t parse_rrq(char *buf, size_t len_buf, char *filename,
                     tftp_mode_t *mode);

// handler function when RRQ packet is received in the parent process
void rrq_handler(char *filename, tftp_mode_t mode,
                 const struct sockaddr *client_addr);

#endif
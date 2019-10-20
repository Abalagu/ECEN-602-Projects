#ifndef LIB_H_
#define LIB_H_

#include "config.h"

/* types of tftp packets */
typedef enum opcode_t {
  RRQ = 1,
  WRQ = 2,
  DATA = 3,
  ACK = 4,
  ERROR = 5
} opcode_t;

// status code of lib function return value
typedef enum { TFTP_OK = 0, TFTP_FAIL = -1 } tftp_err_t;

// error code of ERROR packet
// when return from function, use negative number of corresponding enum
typedef enum error_code_t {
  UNDEFINED = 0,
  FILE_NOT_FOUND = 1,
  ACCESS_VIOLATION = 2,
  DISK_FULL = 3,
  ILLEGAL_OP = 4,
  UNKNOWN_TID = 5,
  FILE_EXISTS = 6,
  NO_SUCH_USER = 7,
} error_code_t;

// define enum for two file type
typedef enum tftp_mode_t { NETASCII = 1, OCTET = 2 } tftp_mode_t;

typedef struct tftp_header_t {
  uint16_t opcode;
  char trail_buf[MAXBUFLEN];
} tftp_header_t;

typedef struct tftp_data_packet_t {
  uint16_t opcode;
  uint16_t block_num;
  char payload[512];
} tftp_data_packet_t;

typedef struct tftp_error_packet_t {
  uint16_t opcode;
  uint16_t error_code;
  char error_msg[128];
  uint8_t trailing_zero; // empty byte from the spec
} tftp_error_packet_t;

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
#ifndef CONFIG_H_
#define CONFIG_H_

#define MYPORT "4950"  // the port users will be connecting to
#define MAXBUFLEN 100

// define opcode
typedef enum {
  RRQ = 1,
  WRQ = 2,
  DATA = 3,
  ACK = 4,
  ERROR = 5,
} opcode_t;

typedef enum {
  TFTP_OK = 0,
  TFTP_ERR = -1,
} tftp_error_t;

typedef enum {
  OCTET = 1,
  NETASCII = 2,
} tftp_mode_t;

#endif
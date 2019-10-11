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
  UNDEFINED = 0,
  FILE_NOT_FOUND = -1,  // use negative number as status code, take negative
                        // value when fill in ERROR packet
  ACCESS_VIOLATION = -2,
  DISK_FULL = -3,
  ILLEGAL_OP = -4,
  UNKNOWN_TID = -5,
  FILE_ALREADY_EXISTS = -6,
  NO_SUCH_USER = -7,
} error_code_t;

typedef enum {
  TFTP_OK = 0,
  TFTP_ERR = -10,  // lib function error code, set to -10 to avoid collision
                   // with error code in ERROR packet
} tftp_error_t;

typedef enum {
  OCTET = 1,
  NETASCII = 2,
} tftp_mode_t;

#endif
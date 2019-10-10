#ifndef CONFIG_H_
#define CONFIG_H_

#define MYPORT "4950"  // the port users will be connecting to
#define MAXBUFLEN 100

// define Mode
#define NETASCII "NETASCII"
#define OCTET "OCTET"
// end of mode define

// define opcode
typedef enum {
  UNDEFINED = -1,
  RRQ = 1,
  WRQ = 2,
  DATA = 3,
  ACK = 4,
  ERROR = 5,
} opcode_t;

typedef enum {
  OK = 0,
  CORRUPT = -1,

} tftp_error_t;
#endif
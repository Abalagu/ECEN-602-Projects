#ifndef CONFIG_H_
#define CONFIG_H_

#define MYPORT "4950"  // the port users will be connecting to
#define MAXBUFLEN 100

typedef enum {
  RRQ = 1,
  WRQ = 2,
  DATA = 3,
  ACK = 4,
  ERROR = 5,
} opcode_t;

#endif
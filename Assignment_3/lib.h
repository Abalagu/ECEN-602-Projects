#ifndef LIB_H_
#define LIB_H_

#include "config.h"

/* types of tftp packets */
typedef enum { RRQ = 1, WRQ = 2, DATA = 3, ACK = 4, ERROR = 5 } opcode_t;

typedef struct {
  uint16_t opcode;
  char trail_buf[MAXBUFLEN];
} tftp_header_t;

typedef struct {
  uint16_t opcode;
  uint16_t block_num;
  char payload[512];
} tftp_data_t;

/* IPv4 or IPv6 */
void *get_in_addr(struct sockaddr *);

/* initialize server */
int init(char *port);

/* header parser*/
opcode_t parse_header(char buf[MAXBUFLEN], char filename[MAXBUFLEN],
                      char mode[8]);

#define DATA_PACKET_LEN 516; // opcode + #block + payload = 2 + 2 + 512 = 516
int read_block(char *filename, char *buf);
void rrq_handler(char *filename, const struct sockaddr *client_addr);
void print_hex(void *array, size_t len); // print buffer in hex
#endif
#ifndef LIB_H_
#define LIB_H_

#include "config.h"

/* types of tftp packets */
typedef enum {
	RRQ = 1,
	WRQ = 2,
	DATA = 3,
	ACK = 4,
	ERROR = 5
} opcode_t;

typedef struct
{
	uint16_t opcode;
	char trail_buf[MAXBUFLEN];
}tftp_header_t;

/* IPv4 or IPv6 */
void *get_in_addr(struct sockaddr *);

/* initialize server */
int init();

#endif
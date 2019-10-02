#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#define VRSN 3

typedef enum
{
    JOIN = 2,
    FWD = 3,
    SEND = 4,
    ACK = 5,
    OFFLINE = 6,
    ACK = 7,
    ONLINE = 8,
    IDLE = 9
} sbcp_header_type_t;

typedef enum
{
    REASON = 1,
    USERNAME = 2,
    CLIENTCOUNT = 3,
    MESSAGE = 4
} sbcp_attribute_type_t;

typedef struct
{
    uint16_t sbcp_attribute_type;
    uint16_t len; //record written payload size
    char payload[512];
} sbcp_attribute_t;

typedef struct
{
    uint32_t vrsn_type_len;            //vrsn: 9 bits; type: 7 bits; len: 16 bits
    sbcp_attribute_t sbcp_attributes[2]; // payload
} sbcp_msg_t;

#endif
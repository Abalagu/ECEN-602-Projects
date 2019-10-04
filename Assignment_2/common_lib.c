#include "common_lib.h"

void print_hex(char *array)
{
    for (int i = 0; i < sizeof array; i++)
    {
        printf(" %2x", array[i]);
    }
    printf("\n");
}

// print multiple username from 1-d buffer
void print_usernames(char *buf)
{
    // each username has 16 bytes space
    char username[16];
    for (int i = 0; i < 16; i++)
    {
        memcpy(username, buf + i * 16, 16);
        if (strlen(username) == 0)
        {
            break;
        }
        else
        {
            printf("user %d: %s\n", i, username);
        }
    }
}

// parse version, type, and length
void parse_vtl(uint32_t vtl)
{
    int version = vtl >> 23;
    int type = (vtl >> 16) & 0b0000000001111111;
    printf("    HEADER: vrsn: %d, type: %d, len: %d\n", version, type, vtl & 0xFFFF);
}

void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute)
{
    if (sbcp_attribute.sbcp_attribute_type == 0)
    {
        printf("    !empty attribute\n");
        return;
    }
    printf("    ATTRIBUTE: %d, %d, %s\n",
           sbcp_attribute.len,
           sbcp_attribute.sbcp_attribute_type,
           sbcp_attribute.payload);
}

void parse_msg_join(sbcp_msg_t msg_join)
{
    printf("sizeof msg: %ld\n", sizeof(msg_join));
    parse_vtl(msg_join.vrsn_type_len);
    parse_sbcp_attribute(msg_join.sbcp_attributes[0]);
    parse_sbcp_attribute(msg_join.sbcp_attributes[1]);
}

void parse_msg_ack(sbcp_msg_t msg_ack)
{
    parse_vtl(msg_ack.vrsn_type_len);
    parse_sbcp_attribute(msg_ack.sbcp_attributes[0]);
    // parse_sbcp_attribute(msg_ack.sbcp_attributes[1]);
    print_usernames(msg_ack.sbcp_attributes[1].payload);
}
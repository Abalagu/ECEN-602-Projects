#include "common_lib.h"

// parse version, type, and length
int parse_vtl(uint32_t vtl)
{
    int version = vtl >> 23;
    int type = (vtl >> 16) & 0b0000000001111111;
    int msg_len = vtl & 0xFFFF;
    printf("sizeof vtl: %ld\n", sizeof(vtl));
    printf("    vrsn: %d\n", version);
    printf("    msg type: %d\n", type);
    printf("    msg len: %d\n", msg_len);

    return msg_len;
}

void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute, size_t total_attr_len)
{
    // printf("sizeof sbcp_attr: %ld\n", sizeof(sbcp_attribute));
    printf("sbcp attribute: \n");
    printf("    attribute len: %d\n", sbcp_attribute.len);
    printf("    attribute type: %d\n", sbcp_attribute.sbcp_attribute_type);
    printf("    attribute payload: %s\n", sbcp_attribute.payload);
}

int parse_msg_join(sbcp_msg_t msg_join)
{
    // printf("sizeof msg: %ld\n", msg_join.);
    int msg_len = parse_vtl(msg_join.vrsn_type_len);
    int attr_len = msg_len - 4;
    // sbcp_attribute_t sbcp_attribute = malloc()
    parse_sbcp_attribute(msg_join.sbcp_attributes[0], attr_len);

    return msg_len;
}

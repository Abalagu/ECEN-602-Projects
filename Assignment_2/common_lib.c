#include "common_lib.h"

// parse version, type, and length
void parse_vtl(uint32_t vtl)
{
    int version = vtl >> 23;
    int type = (vtl >> 16) & 0b0000000001111111;
    printf("sizeof vtl: %ld\n", sizeof(vtl));
    printf("    vrsn: %d\n", version);
    printf("    type: %d\n", type);
    printf("    len: %d\n", vtl & 0xFFFF);
}

void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute)
{
    printf("sizeof sbcp_attr: %ld\n", sizeof(sbcp_attribute));
    printf("    attribute len: %d\n", sbcp_attribute.len);
    printf("    attribute type: %d\n", sbcp_attribute.sbcp_attribute_type);
    printf("    attribute payload: %s\n", sbcp_attribute.payload);
}

void parse_msg_join(sbcp_msg_t msg_join)
{
    printf("sizeof msg: %ld\n", sizeof(msg_join));
    parse_vtl(msg_join.vrsn_type_len);
    parse_sbcp_attribute(msg_join.sbcp_attributes[0]);
}

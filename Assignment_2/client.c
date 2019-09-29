#include "headers.h"
#include "config.h"

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

sbcp_msg_t make_msg_join(char* username)
{
    sbcp_attribute_t *sbcp_attr_join = malloc(sizeof(sbcp_attribute_t));
    sbcp_attr_join->sbcp_attribute_type = USERNAME;
    sbcp_attr_join->payload = malloc(sizeof(username));
    sbcp_attr_join->len = sizeof(*sbcp_attr_join);
    
    strcpy(sbcp_attr_join->payload, username);
    sbcp_msg_t *msg_join = malloc(sizeof(sbcp_msg_t));
    msg_join->sbcp_attributes = sbcp_attr_join;
    msg_join->vrsn_type_len = (VRSN << 23 | JOIN << 16 | sizeof(*msg_join));

    return *msg_join;
}

int main(int argc, char *argv[])
{
    printf("\n");
    sbcp_msg_t msg_join = make_msg_join("luming");
    parse_msg_join(msg_join);
}

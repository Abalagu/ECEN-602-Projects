#include "client_lib.h"

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
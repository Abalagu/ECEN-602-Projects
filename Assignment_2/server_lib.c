#include "server_lib.h"

sbcp_msg_t make_msg_fwd(char *message, size_t msg_len, char *username, size_t name_len)
{
    sbcp_msg_t msg_fwd = {0};
    msg_fwd.vrsn_type_len = (VRSN << 23 | FWD << 16 | sizeof(sbcp_msg_t));

    // fill in message part
    msg_fwd.sbcp_attributes[0].sbcp_attribute_type = MESSAGE;
    msg_fwd.sbcp_attributes[0].len = msg_len;
    memcpy(msg_fwd.sbcp_attributes[0].payload, message, msg_len);

    // fill in username part
    msg_fwd.sbcp_attributes[1].sbcp_attribute_type = USERNAME;
    msg_fwd.sbcp_attributes[1].len = name_len;
    memcpy(msg_fwd.sbcp_attributes[1].payload, username, name_len);

    return msg_fwd;
}


// bonus feature: REASON attribute
sbcp_msg_t make_msg_nak(char *reason, size_t reason_len)
{
    sbcp_msg_t msg_nak = {0};
    msg_nak.vrsn_type_len = (VRSN << 23 | SEND << 16 | sizeof(sbcp_msg_t));

    // fill in reason part
    msg_nak.sbcp_attributes[0].sbcp_attribute_type = REASON;
    msg_nak.sbcp_attributes[0].len = reason_len;
    memcpy(msg_nak.sbcp_attributes[0].payload, reason, reason_len);

    return msg_nak;
}

// bonus feature: OFFLINE attribute
sbcp_msg_t make_msg_offline(char *username, size_t name_len)
{
    sbcp_msg_t msg_offline = {0};
    msg_offline.vrsn_type_len = (VRSN << 23 | OFFLINE << 16 | sizeof(sbcp_msg_t));

    // fill in offline username
    msg_offline.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
    msg_offline.sbcp_attributes[0].len = name_len;
    memcpy(msg_offline.sbcp_attributes[0].payload, username, name_len);

    return msg_offline;
}

// count should be inclusive of the requestor
sbcp_msg_t make_msg_ack(int count, char *clients)
{
    sbcp_msg_t msg_ack = {0};
    char count_str[5];
    sprintf(count_str, "%d", count);
    msg_ack.vrsn_type_len = (VRSN << 23 | ACK << 16 | sizeof(sbcp_msg_t));

    // fill in reason part
    msg_ack.sbcp_attributes[0].sbcp_attribute_type = CLIENTCOUNT;
    msg_ack.sbcp_attributes[0].len = sizeof(count_str);
    memcpy(msg_ack.sbcp_attributes[0].payload, count_str, sizeof(count_str));

    return msg_ack;
}

sbcp_msg_t make_msg_online(char *username, size_t name_len)
{
    sbcp_msg_t msg_online = {0};
    msg_online.vrsn_type_len = (VRSN << 23 | ONLINE << 16 | sizeof(sbcp_msg_t));

    // fill in online username to forward
    msg_online.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
    msg_online.sbcp_attributes[0].len = name_len;
    memcpy(msg_online.sbcp_attributes[0].payload, username, name_len);

    return msg_online;
}


// idle message from server fwd to clients
sbcp_msg_t make_msg_idle_s(char *username, size_t name_len)
{
    sbcp_msg_t msg_idle = {0};
    msg_idle.vrsn_type_len = (VRSN << 23 | IDLE << 16 | sizeof(sbcp_msg_t));
    // msg with empty attributes

    return msg_idle;
}

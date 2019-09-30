#include "client.h"

int main(int argc, char *argv[])
{
    printf("\n");
    sbcp_msg_t msg_join = make_msg_join("luming");
    parse_msg_join(msg_join);
}

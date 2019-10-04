#include "common_lib.h"

<<<<<<< HEAD
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
=======
void print_hex(char *array) {
  for (int i = 0; i < sizeof array; i++) {
    printf(" %2x", array[i]);
  }
  printf("\n");
}

// get msg type from header
int get_msg_type(sbcp_msg_t msg) {
  int type = (msg.vrsn_type_len >> 16) & 0b0000000001111111;

  return type;
}

// parse version, type, and length
void parse_vtl(uint32_t vtl) {
  int version = vtl >> 23;
  int type = (vtl >> 16) & 0b0000000001111111;
  printf("    HEADER: vrsn: %d, type: %d, len: %d\n", version, type,
         vtl & 0xFFFF);
}

void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute) {
  if (sbcp_attribute.sbcp_attribute_type == 0) {
    printf("    !empty attribute\n");
    return;
  }
  printf("    ATTRIBUTE: %d, %d, %s\n", sbcp_attribute.len,
         sbcp_attribute.sbcp_attribute_type, sbcp_attribute.payload);
>>>>>>> b2f8ac1901784d4524198d4da2df1d3d74e412b8
}

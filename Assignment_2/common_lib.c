#include "common_lib.h"

// from SO, check if any fd is set
bool FD_IS_ANY_SET(fd_set const *fdset) {
  static fd_set empty;
  return memcmp(fdset, &empty, sizeof(fd_set)) != 0;
}

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
}

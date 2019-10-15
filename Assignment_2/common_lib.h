#ifndef COMMON_LIB_H_
#define COMMON_LIB_H_

#include "config.h"
#include "headers.h"
#define STDIN 0  // file descriptor for stdin

bool FD_IS_ANY_SET(fd_set const *fdset);

void print_hex(char *array);

// get header from msg
int get_msg_type(sbcp_msg_t msg);

// parse version, type, and length
void parse_vtl(uint32_t vtl);

void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute);

#endif

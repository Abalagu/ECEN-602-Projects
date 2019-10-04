#ifndef COMMON_LIB_H_
#define COMMON_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"

#endif

void print_hex(char *array);

// get header from msg
int get_msg_type(sbcp_msg_t msg);

// parse version, type, and length
int parse_vtl(uint32_t vtl);

<<<<<<< HEAD
void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute, size_t attr_len);

int parse_msg_join(sbcp_msg_t msg_join);
=======
void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute);
>>>>>>> b2f8ac1901784d4524198d4da2df1d3d74e412b8

#ifndef COMMON_LIB_H_
#define COMMON_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>

#include "config.h"

#endif

void print_hex(char *array);

void print_usernames(char *buf);

// parse version, type, and length
void parse_vtl(uint32_t vtl);

void parse_sbcp_attribute(sbcp_attribute_t sbcp_attribute);

void parse_msg_join(sbcp_msg_t msg_join);

void parse_msg_ack(sbcp_msg_t msg_ack);

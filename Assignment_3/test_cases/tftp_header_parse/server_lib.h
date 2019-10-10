#ifndef SERVER_LIB_H_
#define SERVER_LIB_H_

#include "headers.h"

// void print_hex(char *array, size_t len);
void print_hex(void *array, size_t len);

void *get_in_addr(struct sockaddr *sa);

int launch_udp_server();

opcode_t get_type(char *packet, size_t packet_len);

#endif

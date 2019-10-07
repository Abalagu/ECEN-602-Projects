#ifndef CLIENT_LIB_H_
#define CLIENT_LIB_H_

#include "common_lib.h"
#include "config.h"

void print_usernames(char *buf);

sbcp_msg_t make_msg_join(char *username, size_t payload_len);

sbcp_msg_t make_msg_send(char *message, size_t msg_len);

// idle message from client to server
sbcp_msg_t make_msg_idle_c(char *username, size_t name_len);

void parse_msg_nak(sbcp_msg_t msg_nak);

int parse_msg_ack(sbcp_msg_t msg_ack, char *username);

void parse_msg_fwd(sbcp_msg_t msg_fwd);

void parse_msg_offline(sbcp_msg_t msg_offline);

// int writen(int sockfd, char *buf);
int writen(int sockfd, char *buf, size_t size_buf);

int readline(int sockfd, char *recvbuf);

int server_lookup_connect(char *host, char *server_port);

// print multiple username from 1-d buffer
// void print_usernames(char *buf);

#endif
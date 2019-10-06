#ifndef SERVER_LIB_H_
#define SERVER_LIB_H_

#include "common_lib.h"
#include "config.h"
#define BACKLOG 10

#endif

// A linked list node
typedef struct socket_fd_t {
  int fd;
  char username[16];
  struct socket_fd_t *next;
} socket_fd_t;

void append_node(socket_fd_t **head_ref, int new_fd, char *new_username);

void print_nodes(socket_fd_t *node);

sbcp_msg_t make_msg_fwd(char *message, size_t msg_len, char *username,
                        size_t name_len);

sbcp_msg_t make_msg_nak(char *reason, size_t reason_len);

sbcp_msg_t make_msg_offline(char *username, size_t name_len);

sbcp_msg_t make_msg_ack(int count, char usernames[10][16]);

sbcp_msg_t make_msg_online(char *username, size_t name_len);

sbcp_msg_t make_msg_idle_s(char *username, size_t name_len);

char *str_join(char *buf, char string_array[10][16]);

void parse_msg_join(sbcp_msg_t msg_join);

void parse_msg_send(sbcp_msg_t msg_send);

// connection rxtx related
void sigchild_handler(int s);

void *get_in_addr(struct sockaddr *sa);

int server_read(int new_fd, char *buf);

int server_write(int new_fd, char *buf);

// refactored server init bind and receive connection
int server_init(char *port);

int connect_client(int sockfd);

#include "server_lib.h"

//-------- SOCKET FD MANAGEMENT --------

void fd_select(fd_set *readfds, socket_fd_t listen_fd) {
  int max_fd;
  struct timeval tv;
  FD_ZERO(readfds);
  FD_SET(listen_fd.fd, readfds);

  tv.tv_sec = 5;
  tv.tv_usec = 500000;
  socket_fd_t *node = &listen_fd;

  // traverse through all fds, get max fd and call select
  while (node != NULL) {
    FD_SET(node->fd, readfds);
    max_fd = (max_fd > node->fd) ? max_fd : node->fd;
    node = node->next;
  }
  select(max_fd + 1, readfds, NULL, NULL, &tv);
}

/* Given a reference (pointer to pointer) to the head
of a list and an int, appends a new node at the end */
void append_node(socket_fd_t **head_ref, int new_fd, char *new_username) {
  /* 1. allocate node */
  socket_fd_t *new_node = (socket_fd_t *)malloc(sizeof(socket_fd_t));
  socket_fd_t *last = *head_ref; /* used in step 5*/

  /* 2. put in the data */
  new_node->fd = new_fd;
  memcpy(new_node->username, new_username, strlen(new_username));

  /* 3. This new node is going to be the last node, so make next of
          it as NULL*/
  new_node->next = NULL;

  /* 4. If the Linked List is empty, then make the new node as head */
  if (*head_ref == NULL) {
    *head_ref = new_node;
    return;
  }

  /* 5. Else traverse till the last node */
  while (last->next != NULL) last = last->next;

  /* 6. Change the next of last node */
  last->next = new_node;
  return;
}

// This function prints contents of linked list starting from head
void print_nodes(socket_fd_t *node) {
  printf("  Monitored Nodes: \n");
  while (node != NULL) {
    printf("    %d: %s\n", node->fd, node->username);
    node = node->next;
  }
}

//-------- END OF SOCKET FD MANAGEMENT --------

// join 2-d username array to 1-d array of size 512
char *str_join(char *buf, char string_array[10][16]) {
  // assume buffer has 512 size
  for (int i = 0; i < 16; i++) {
    memcpy(buf + i * 16, string_array[i], 16);
  }
  return buf;
}

sbcp_msg_t make_msg_fwd(char *message, size_t msg_len, char *username,
                        size_t name_len) {
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
sbcp_msg_t make_msg_nak(char *reason, size_t reason_len) {
  sbcp_msg_t msg_nak = {0};
  msg_nak.vrsn_type_len = (VRSN << 23 | NAK << 16 | sizeof(sbcp_msg_t));

  // fill in reason part
  msg_nak.sbcp_attributes[0].sbcp_attribute_type = REASON;
  msg_nak.sbcp_attributes[0].len = reason_len;
  memcpy(msg_nak.sbcp_attributes[0].payload, reason, reason_len);

  return msg_nak;
}

// bonus feature: OFFLINE attribute
sbcp_msg_t make_msg_offline(char *username, size_t name_len) {
  sbcp_msg_t msg_offline = {0};
  msg_offline.vrsn_type_len = (VRSN << 23 | OFFLINE << 16 | sizeof(sbcp_msg_t));

  // fill in offline username
  msg_offline.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
  msg_offline.sbcp_attributes[0].len = name_len;
  memcpy(msg_offline.sbcp_attributes[0].payload, username, name_len);

  return msg_offline;
}

// count should be inclusive of the requestor
sbcp_msg_t make_msg_ack(int count, char *usernames) {
  sbcp_msg_t msg_ack = {0};
  msg_ack.vrsn_type_len = (VRSN << 23 | ACK << 16 | sizeof(sbcp_msg_t));

  // fill in client count
  char count_str[5];
  sprintf(count_str, "%d", count);
  msg_ack.sbcp_attributes[0].sbcp_attribute_type = CLIENTCOUNT;
  msg_ack.sbcp_attributes[0].len = sizeof(count_str);
  memcpy(msg_ack.sbcp_attributes[0].payload, count_str, sizeof(count_str));

  // fill in client names
  msg_ack.sbcp_attributes[1].sbcp_attribute_type = USERNAME;
  msg_ack.sbcp_attributes[1].len = sizeof(160);
  memcpy(msg_ack.sbcp_attributes[1].payload, usernames, 170);

  return msg_ack;
}

sbcp_msg_t make_msg_online(char *username, size_t name_len) {
  sbcp_msg_t msg_online = {0};
  msg_online.vrsn_type_len = (VRSN << 23 | ONLINE << 16 | sizeof(sbcp_msg_t));

  // fill in online username to forward
  msg_online.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
  msg_online.sbcp_attributes[0].len = name_len;
  memcpy(msg_online.sbcp_attributes[0].payload, username, name_len);

  return msg_online;
}

// idle message with username from server fwd to clients
sbcp_msg_t make_msg_idle_s(char *username, size_t name_len) {
  sbcp_msg_t msg_idle = {0};
  msg_idle.vrsn_type_len = (VRSN << 23 | IDLE << 16 | sizeof(sbcp_msg_t));
  // fill in username
  msg_idle.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
  msg_idle.sbcp_attributes[0].len = name_len;
  memcpy(msg_idle.sbcp_attributes[0].payload, username, name_len);
  return msg_idle;
}

void parse_msg_join(sbcp_msg_t msg_join, char *new_name) {
  if (msg_join.sbcp_attributes[0].sbcp_attribute_type == USERNAME) {
    memcpy(new_name, msg_join.sbcp_attributes[0].payload, 16);
    printf("%s want to join the chat.\n", new_name);
  }
}

void parse_msg_idle(sbcp_msg_t msg_idle) {}

void parse_msg_send(sbcp_msg_t msg_send, char *client_message) {
  memcpy(client_message, msg_send.sbcp_attributes[0].payload, 512);
  // printf("msg: %s\n", );
}

// flatten usernames in nodes to char array
void get_usernames(char *usernames, socket_fd_t *listen_fd) {
  memset(usernames, 0, sizeof(usernames));
  socket_fd_t *node = listen_fd->next;
  int count = 0;
  while (node != NULL) {
    memcpy(usernames + count * 16, node->username, 16);
    count += 1;
    node = node->next;
  }
}

// remove node from linked list of socket fds
void remove_node(socket_fd_t *listen_fd, socket_fd_t remove_node) {
  socket_fd_t *node = listen_fd;
  while (node != NULL) {
    // if next node is to be removed
    if (node->next->fd == remove_node.fd) {
      // link node next to remove node to its parent
      node->next = remove_node.next;
      return;
    } else {
      node = node->next;
    }
  }
}

// given current node, head node, and message, broadcast message to all other
// nodes
void msg_broadcast(socket_fd_t *current_node, socket_fd_t *head,
                   sbcp_msg_t *msg_recv) {
  char buf[MAXDATASIZE];
  char client_chat[512] = {0};

  parse_msg_send(*msg_recv, client_chat);
  printf("%s: %s\n", current_node->username, client_chat);
  sbcp_msg_t msg_send = make_msg_fwd(client_chat, 512, current_node->username,
                                     strlen(current_node->username) + 1);

  memcpy(buf, &msg_send, sizeof(sbcp_msg_t));

  // record current node fd, skip sending to message origin
  // head node is server
  int message_origin_fd = current_node->fd;
  socket_fd_t *node = head->next;
  while (node != NULL) {
    if (node->fd != message_origin_fd) {  // if not from origin
      server_write(node->fd, buf);        // broadcast to others
    }
    node = node->next;
  }
}

int is_duplicate_name(socket_fd_t *listen_fd, char *new_name) {
  socket_fd_t *node = listen_fd->next;
  while (node != NULL) {
    if (strcmp(node->username, new_name) == 0) {  // duplicate detected.
      return 1;
    }
    node = node->next;
  }

  return 0;
}

// traverse through all nodes, recv possible msg
void msg_router(socket_fd_t *listen_fd, fd_set readfds) {
  char buf[MAXDATASIZE];
  int new_fd;
  int numbytes, msg_type;
  char usernames[512] = {0};
  char new_name[16];
  int message_origin_fd = -1;
  char reason[] = "same username";

  sbcp_msg_t msg_send, *msg_recv;

  if (FD_ISSET(listen_fd->fd, &readfds)) {  // incoming new connection
    new_fd = connect_client(listen_fd->fd);
    append_node(&listen_fd, new_fd, "");
  }

  socket_fd_t *node = listen_fd->next;
  while (node != NULL) {
    if (FD_ISSET(node->fd, &readfds)) {  // a client sends msg
      numbytes = server_read(node->fd, buf);
      if (numbytes == 0) {
        printf("FIN received.\n");
        close(node->fd);  // handle disconnection. should remove from node
        remove_node(listen_fd, *node);
        // return;  // temporary handle of disconnection
      }

      // cast buffer to message
      msg_recv = (sbcp_msg_t *)buf;
      msg_type = get_msg_type(*msg_recv);
      if (msg_type == SEND) {  // msg send, fwd to others

        msg_broadcast(node, listen_fd, msg_recv);
      }

      if (msg_type == JOIN) {  // msg join, add to node
        parse_msg_join(*msg_recv, new_name);
        if (is_duplicate_name(listen_fd, new_name) == 0) {
          printf("%s ACCEPTED.\n", new_name);
          memcpy(node->username, new_name, 16);
          get_usernames(usernames, listen_fd);
          msg_send = make_msg_ack(1, usernames);
          memcpy(buf, &msg_send, sizeof(sbcp_msg_t));  // SEND ACK TEST
          numbytes = server_write(node->fd, buf);
        } else {  // is duplicate, send NAK, close fd, then remove this node
          printf("%s REJECTED!\n", new_name);
          msg_send = make_msg_nak(reason, sizeof(reason));
          memcpy(buf, &msg_send, sizeof(msg_send));  // SEND NAK TEST
          close(node->fd);
          remove_node(listen_fd, *node);
        
        };
      }
    }  // if fd in node is set
    node = node->next;
  }  // client node traversal
}

// function taken from beej's guide
void sigchild_handler(int s) {
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int server_read(int new_fd, char *buf) {
  // read the received buffer from the socket
  return recv(new_fd, buf, MAXDATASIZE - 1, 0);
}

int server_write(int new_fd, char *buf) {
  // send the buffer to the socket
  return send(new_fd, buf, MAXDATASIZE - 1, 0);
}

int server_init(char *port) {
  struct sigaction sa;
  int yes = 1;

  struct addrinfo hints, *res, *p;
  int status;
  int sockfd;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;  // fill in my IP

  if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }

  // loop through all the results and bind to the first correct
  for (p = res; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    // allow other sockets to bind to this port
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
      perror("setsocketopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }
  // we don't need it now
  freeaddrinfo(res);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  // reap all dead process - function taken from beej's guide
  sa.sa_handler = sigchild_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections....\n");
  return sockfd;
}

int connect_client(int sockfd) {
  int new_fd;
  struct sockaddr_storage their_addr;  // connector's address information

  int sin_size = sizeof(their_addr);
  char str[sin_size];
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
            str, sin_size);

  // outside while loop
  // printf("server: got conection from %s\n", str);
  return new_fd;
}
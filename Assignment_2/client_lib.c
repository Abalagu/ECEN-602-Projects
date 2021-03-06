#include "client_lib.h"

// add pasted time to timer
int update_idle_time(int idle_cumulation, struct timeval tv, int is_idle) {
  if (!is_idle) {
    int elapsed_time =
        (SELECT_TIMEOUT * 1000000 - (tv.tv_usec + 1000000 * tv.tv_sec));

    // tolerance deduct elapsed time
    idle_cumulation += elapsed_time;
    // printf("total idle time: %d\n", idle_cumulation);
  }
  return idle_cumulation;
}
// print multiple username from 1-d buffer
void print_usernames(char *buf) {
  // each username has 16 bytes space
  char username[16];
  for (int i = 0; i < 16; i++) {
    memcpy(username, buf + i * 16, 16);
    if (strlen(username) == 0) {
      break;
    } else {
      printf("user %d: %s\n", i, username);
    }
  }
}

sbcp_msg_t make_msg_join(char *username, size_t name_len) {
  sbcp_msg_t msg_join = {0};
  msg_join.vrsn_type_len = (VRSN << 23 | JOIN << 16 | sizeof(sbcp_msg_t));

  // fill in message part
  msg_join.sbcp_attributes[0].sbcp_attribute_type = USERNAME;
  msg_join.sbcp_attributes[0].len = name_len;
  memcpy(msg_join.sbcp_attributes[0].payload, username, name_len);

  return msg_join;
}

sbcp_msg_t make_msg_send(char *message, size_t msg_len) {
  sbcp_msg_t msg_send = {0};
  msg_send.vrsn_type_len = (VRSN << 23 | SEND << 16 | sizeof(sbcp_msg_t));

  // fill in message part
  msg_send.sbcp_attributes[0].sbcp_attribute_type = MESSAGE;
  msg_send.sbcp_attributes[0].len = msg_len;
  memcpy(msg_send.sbcp_attributes[0].payload, message, msg_len);

  return msg_send;
}

// idle message with empty attribute from client to server
sbcp_msg_t make_msg_idle_c(char *username, size_t name_len) {
  sbcp_msg_t msg_idle = {0};
  msg_idle.vrsn_type_len = (VRSN << 23 | IDLE << 16 | sizeof(sbcp_msg_t));

  // msg with empty attributes

  return msg_idle;
}

void parse_msg_fwd(sbcp_msg_t msg_fwd) {
  if (msg_fwd.sbcp_attributes[0].sbcp_attribute_type == MESSAGE &&
      msg_fwd.sbcp_attributes[1].sbcp_attribute_type == USERNAME) {
    // username: message
    printf("%s: %s\n", msg_fwd.sbcp_attributes[1].payload,
           msg_fwd.sbcp_attributes[0].payload);
  } else {
    printf("!WRONG ATTRIBUTE TYPE FOR MSG FWD.\n");
  }
}

void parse_msg_nak(sbcp_msg_t msg_nak) {
  if (msg_nak.sbcp_attributes[0].sbcp_attribute_type == REASON) {
    printf("Join rejected! NAK REASON: %s\n",
           msg_nak.sbcp_attributes[0].payload);
  } else {
    printf("ATTRIBUTE ERROR. EXPECT REASON.\n");
  }
}

int parse_msg_ack(sbcp_msg_t msg_ack, char *my_name) {
  if (msg_ack.sbcp_attributes[0].sbcp_attribute_type == CLIENTCOUNT &&
      msg_ack.sbcp_attributes[1].sbcp_attribute_type == USERNAME) {
    printf("Join success! Your username: %s\n", my_name);
    printf("#clients in chat(excluding yourself): %s\n",
           msg_ack.sbcp_attributes[0].payload);
    return 0;
  } else {  // attribute error
    return 1;
  }
}

void parse_msg_online(sbcp_msg_t msg_online) {
  if (msg_online.sbcp_attributes[0].sbcp_attribute_type == USERNAME) {
    printf("%s has joined the chat.\n", msg_online.sbcp_attributes[0].payload);
  } else {
    printf("MSG ONLINE ATTR ERROR.\n");
  }
}

void parse_msg_offline(sbcp_msg_t msg_offline) {
  if (msg_offline.sbcp_attributes[0].sbcp_attribute_type == USERNAME) {
    printf("%s has left the chat.\n", msg_offline.sbcp_attributes[0].payload);
  } else {
    printf("MSG OFFLINE ATTR ERROR.\n");
  }
}

void parse_msg_idle(sbcp_msg_t msg_idle) {
  printf("%s is idle.\n", msg_idle.sbcp_attributes[0].payload);
}
int writen(int sockfd, char *buf, size_t size_buf) {
  int numbytes;
  while ((numbytes = send(sockfd, buf, size_buf, 0)) == -1 && errno == EINTR) {
    // manually restarting
    continue;
  }
  return numbytes;
}

int readline(int sockfd, char *recvbuf) {
  int numbytes;
  while ((numbytes = recv(sockfd, recvbuf, MAXDATASIZE - 1, 0)) == -1 &&
         errno == EINTR) {
    // manually restarting
  }
  return numbytes;
}

int server_lookup_connect(char *host, char *server_port) {
  struct addrinfo hints, *server_info, *p;
  int status;
  int sock_fd;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  // hints.ai_flags = AI_PASSIVE;     // fill in my IP

  // argv[1]: IPAdr
  // argv[2]: Port
  if ((status = getaddrinfo(host, server_port, &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return -2;
  }

  for (p = server_info; p != NULL; p = p->ai_next) {  // loop through link list
    sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sock_fd == -1) {  // socket creation failed
      perror("client: socket");
      continue;
    }

    if (connect(sock_fd, p->ai_addr, p->ai_addrlen) ==
        -1) {  // connection failed
      close(sock_fd);
      perror("client: connect");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return -2;
  }

  printf("client: connected to %s:%s\n", host, server_port);

  freeaddrinfo(server_info);
  return sock_fd;
}
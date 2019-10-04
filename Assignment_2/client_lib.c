#include "client_lib.h"

<<<<<<< HEAD
// len for payload length
sbcp_msg_t make_msg_join(char *username, size_t len)
{
    sbcp_attribute_t *sbcp_attr_join = malloc(sizeof(sbcp_attribute_t) + len);
    sbcp_attr_join->sbcp_attribute_type = USERNAME;
    memcpy(sbcp_attr_join->payload, username, len);
    sbcp_attr_join->len = len + 4; //payload + 2 + 2

    sbcp_msg_t *msg_join = malloc(sbcp_attr_join->len + sizeof(*sbcp_attr_join));
    memcpy(msg_join->sbcp_attributes, sbcp_attr_join, sbcp_attr_join->len);
    // msg_join->sbcp_attributes = sbcp_attr_join;
    msg_join->vrsn_type_len = (VRSN << 23 | JOIN << 16 | sbcp_attr_join->len + 4);
=======
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
>>>>>>> b2f8ac1901784d4524198d4da2df1d3d74e412b8

  return msg_join;
}

<<<<<<< HEAD
int writen(int sockfd, char *buf, size_t size_buf)
{
    int numbytes;
    while ((numbytes = send(sockfd, buf, size_buf, 0)) == -1 && errno == EINTR)
    {
        // manually restarting
        continue;
    }
    return numbytes;
=======
sbcp_msg_t make_msg_send(char *message, size_t msg_len) {
  sbcp_msg_t msg_send = {0};
  msg_send.vrsn_type_len = (VRSN << 23 | SEND << 16 | sizeof(sbcp_msg_t));

  // fill in message part
  msg_send.sbcp_attributes[0].sbcp_attribute_type = MESSAGE;
  msg_send.sbcp_attributes[0].len = msg_len;
  memcpy(msg_send.sbcp_attributes[0].payload, message, msg_len);

  return msg_send;
>>>>>>> b2f8ac1901784d4524198d4da2df1d3d74e412b8
}

// idle message with empty attribute from client to server
sbcp_msg_t make_msg_idle_c(char *username, size_t name_len) {
  sbcp_msg_t msg_idle = {0};
  msg_idle.vrsn_type_len = (VRSN << 23 | IDLE << 16 | sizeof(sbcp_msg_t));

  // msg with empty attributes

  return msg_idle;
}

void parse_msg_nak(sbcp_msg_t msg_nak) {
  if (msg_nak.sbcp_attributes[0].sbcp_attribute_type == REASON) {
    printf("NAK REASON: %s\n", msg_nak.sbcp_attributes[0].payload);
  } else {
    printf("ATTRIBUTE ERROR. EXPECT REASON.\n");
  }
}

void parse_msg_offline(sbcp_msg_t msg_offline) {
  printf("user %s is offline.\n", msg_offline.sbcp_attributes[0].payload);
}

void parse_msg_ack(sbcp_msg_t msg_ack) {
  if (msg_ack.sbcp_attributes[0].sbcp_attribute_type == CLIENTCOUNT) {
    printf("client count(excluding self): %s\n",
           msg_ack.sbcp_attributes[0].payload);
  } else {
    printf("ATTRIBUTE ERROR. EXPECT COUNT.\n");
    return;
  }
  if (msg_ack.sbcp_attributes[1].sbcp_attribute_type == USERNAME) {
    print_usernames(msg_ack.sbcp_attributes[1].payload);
  } else {
    printf("ATTRIBUTE ERROR. EXPECT USERNAME.\n");
    return;
  }
}

void parse_msg_online(sbcp_msg_t msg_online) {
  printf("user %s is back online.\n", msg_online.sbcp_attributes[0].payload);
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

#include "lib.h"

// given sockfd, return local port
http_err_t get_sock_port(int sockfd, int *local_port) {
  struct sockaddr_in sin;
  int addrlen = sizeof(sin);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &addrlen) == 0 &&
      sin.sin_family == AF_INET && addrlen == sizeof(sin)) {
    *local_port = ntohs(sin.sin_port);
    return HTTP_OK;
  } else {
    printf("find port error.\n");
    return HTTP_FAIL;
  }
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

http_err_t server_lookup_connect(char *host, char *server_port, int *sock_fd) {
  struct addrinfo hints, *server_info, *p;
  int status;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // argv[1]: IPAdr
  // argv[2]: Port
  if ((status = getaddrinfo(host, server_port, &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return HTTP_FAIL;
  }

  for (p = server_info; p != NULL; p = p->ai_next) { // loop through link list
    *sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (*sock_fd == -1) { // socket creation failed
      perror("client: socket");
      continue;
    }

    if (connect(*sock_fd, p->ai_addr, p->ai_addrlen) ==
        -1) { // connection failed
      close(*sock_fd);
      perror("client: connect");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return HTTP_FAIL;
  }

  printf("connected to %s:%s\n", host, server_port);

  freeaddrinfo(server_info);
  return HTTP_OK;
}

int written(int sockfd, char *buf, size_t size_buf) {
  int numbytes;
  while ((numbytes = send(sockfd, buf, size_buf, 0)) == -1 && errno == EINTR) {
    // manually restarting
    continue;
  }
  return numbytes;
}

int readline(int sockfd, char *recvbuf) {
  int numbytes;
  while ((numbytes = recv(sockfd, recvbuf, MAX_DATA_SIZE - 1, 0)) == -1 &&
         errno == EINTR) {
    // manually restarting
  }
  return numbytes;
}

// print buffer of given length in hexadecimal form, prefix with 0
void print_hex(void *array, size_t len) {
  char *parray = array;
  for (int i = 0; i < len; i++) {
    printf(" %02x", parray[i]);
  }
  printf("\n");
}

// given listening socket, accept possible client
http_err_t accept_client(int listen_fd, int *client_fd) {
  struct sockaddr_storage their_addr; // connector's address information

  int sin_size = sizeof(their_addr);
  char str[sin_size];
  *client_fd = accept(listen_fd, (struct sockaddr *)&their_addr, &sin_size);

  if(*client_fd <0){
    perror("accept_client");
    return HTTP_FAIL;
  }

  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
            str, sin_size);

  printf("server: got conection from %s\n", str);
  return HTTP_OK;
}

// launch STREAM socket at given port. port = "" results in ephemeral port.
http_err_t server_init(char *port, int *sockfd) {
  int rv, numbytes;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return HTTP_FAIL;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("listener: socket");
      continue;
    }
    if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }
  if (p == NULL) {
    return HTTP_FAIL;
  }
  listen(*sockfd, BACKLOG);
  freeaddrinfo(servinfo);
  int local_port;
  get_sock_port(*sockfd, &local_port);
  printf("HTTP Proxy Server Port: %d\n", local_port);
  return HTTP_OK;
}



void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}


void parse_request(char req_buf[1500], request_t *req) {
	// char url[1500]= "GET /somefile HTTP/1.0\r\nHost: www.go.com\r\nUser-Agent: Team4\r\n\r\n";
	// fix size
  char buf[1500] = {0};

  // separate the first line which contains the path
  char *loc = strchr(req_buf, '\r');
  int pos = (loc == NULL ? -1 : loc - req_buf);
  
  // First 4 and last 13 contain standard HTTP version info, not needed
  int _pos = pos - 13;
  strncpy(req->path, &req_buf[4], _pos);

	// rest of the header
  memcpy(buf, &(req_buf[pos+2]), 1500*sizeof(char));

  char* token;
  char* _buf = buf;
  char* key = malloc(strlen(buf));
  char* value = malloc(strlen(buf));

	// \r\n is the delim here according to spec
  while((token = strtok_r(_buf, "\r\n", &_buf))) {
    // printf("token:%s\n", token);
    loc = strchr(token, ' ');
    pos = (loc == NULL ? -1 : loc - token);
    
    strncpy(key, token, pos);
    strncpy(value, &(token[pos+1]), strlen(token));
    
    if (!strcmp(key, "Host:")) {
      strcpy(req->host, value);
    } else if (!strcmp(key, "User-Agent:")) {
      strcpy(req->user_agent, value);
    } else if (!strcmp(key, "Connection:")) {
			strcpy(req->connection, value);
		}
  }
}

void parse_response(char res_buf[1500], response_t *res) {
	char buf[1500] = {0};
	
	char *loc = strchr(res_buf, '\r');
  int pos = (loc == NULL ? -1 : loc - res_buf);

	// status code does not begin until 9
	memcpy(res->status, &(res_buf[9]), pos-9);

	// copy rest of the buffer
  memcpy(buf, &(res_buf[pos+2]), 1500*sizeof(char));
	if (DEBUG)
  	printf("buf:%s\n", buf);

  char* token;
  char* _buf = buf;
  char* key = malloc(strlen(buf));
  char* value = malloc(strlen(buf));

	// the delim is \r\n
  while((token = strtok_r(_buf, "\r\n", &_buf))) {
    // printf("token:%s\n", token);
    loc = strchr(token, ' ');
    pos = (loc == NULL ? -1 : loc - token);

    if (pos < 0) {
			// EOF
        break;
    }

    strncpy(key, token, pos);
    strncpy(value, &(token[pos+1]), strlen(token));
    
    if (!strcmp(key, "Content-Length:")) {
      strcpy(res->content_length, value);
    } else if (!strcmp(key, "Date:")) {
      strcpy(res->date, value);
    }
		// add more fields which we are interested in
  }
}
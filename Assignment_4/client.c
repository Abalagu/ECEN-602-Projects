#include "headers.h"
#include "lib.h"

http_err_t main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("usage: ./client <proxy address> <proxy port> <URL to rerieve>\n");
    return HTTP_FAIL;
  }
  http_err_t retval;

  // TODO:: validate args

  char *proxy_address = argv[1];
  char *proxy_port = argv[2];
  char *url = argv[3];
  int numbytes = 0;
  int sockfd = 0;

  // this client supports all the possbile formats
  //  "www.cpluscplus.com";
  //  "www.cpluscplus.com/some";
  //  "https://www.cpluscplus.com";
  //  "https://www.cpluscplus.com/some";

  printf("address: %s, port: %s, target: %s\n", proxy_address, proxy_port, url);

  int _size = strlen(argv[3]);

  url = malloc(_size * sizeof(char));
  memcpy(url, argv[3], _size);

  if (server_lookup_connect(proxy_address, proxy_port, &sockfd) != HTTP_OK) {
    // return HTTP_FAIL;
  }

  char *path = malloc(_size * sizeof(char));
  char *host = malloc(_size * sizeof(char));
  char *_u = malloc(_size * sizeof(char));
  char *_p = malloc(_size * sizeof(char));
  char *loc;

  char buf_send[1500] = {0};
  char buf_recv[1500] = {0};
  char request[1500] = {0};

  loc = strchr(url, '/');
  int pos = (loc == NULL ? -1 : loc - url);

  // to fallback in case passed url does not start with https/http
  memcpy(_u, url, _size);

  if (pos > 0) {
    memcpy(_p, url, pos);
  }

  // if url began with https/http
  if (!strcmp(_p, "http:") || !strcmp(_p, "https:")) {
    // update _u
    memcpy(_u, &(url[pos + 2]), _size);
    // look for path in the rest
    loc = strchr(_u, '/');
    pos = (loc == NULL ? -1 : loc - _u);
  }

  if (pos > 0) {
    // we have our host address
    memcpy(host, _u, pos);
    memcpy(path, &(_u[pos]), _size);
  } else {
    // fall back to previous safe values
    strcpy(host, _u);
    strcpy(path, "/");
  }

  // add more fields if required
  sprintf(request, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Team4\r\n\r\n",
          path, host);
  printf("request:\n%s\n", request);
  memcpy(buf_send, request, sizeof(request));
  numbytes = written(sockfd, request, strlen(request)+1);
  printf("sent %d bytes\n", numbytes);
  numbytes = readline(sockfd, buf_recv, MAX_DATA_SIZE);
  printf("\n\nResponse:\n%s\n", buf_recv);
}

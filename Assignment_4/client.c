#include "headers.h"

http_err_t main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("usage: ./client <proxy address> <proxy port> <URL to rerieve>\n");
    return HTTP_FAIL;
  }
  http_err_t retval;

  char *proxy_address = argv[1];
  char *proxy_port = argv[2];
  char *target_url = argv[3];
  char request_line[] = "GET /get HTTP/1.0\r\n\r\n";
  int numbytes = 0;
  printf("address: %s, port: %s, target: %s\n", proxy_address, proxy_port,
         target_url);
  int sockfd = 0;
  if (server_lookup_connect(proxy_address, proxy_port, &sockfd) != HTTP_OK) {
    return HTTP_FAIL;
  }
  char buf_send[1500] = {0};
  char buf_recv[1500] = {0};
  memcpy(buf_send, request_line, sizeof(request_line));
  numbytes = writen(sockfd, request_line, sizeof(request_line));
  numbytes = readline(sockfd, buf_recv);
  printf("proxy return:\n%s\n", buf_recv);
}

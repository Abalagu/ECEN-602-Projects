#include "headers.h"

http_err_t main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("usage: ./client <proxy address> <proxy port> <URL to rerieve>\n");
    return HTTP_FAIL;
  }

  char *proxy_address = argv[1];
  char *proxy_port = argv[2];
  char *target_url = argv[3];
  // int sockfd = server_lookup_connect(proxy_address, proxy_port);
  printf("address: %s, port: %s, target: %s\n", proxy_address, proxy_port,
         target_url);
  server_lookup_connect(proxy_address, proxy_port);
}

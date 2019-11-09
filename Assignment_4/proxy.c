#include "headers.h"

http_err_t main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: ./proxy <ip to bind> <port to bind>\n");
    return HTTP_FAIL;
  }

  char request_line[] = "GET /get HTTP/1.0\r\n\r\n";
  printf("%s\n", request_line);
  char target_site[] = "httpbin.org";

  // char target_site[] = "wikipedia.org";

  char *local_ip = argv[1]; // why specify ip other than localhost?
  char *local_port = argv[2];
  int listen_fd, client_fd, server_fd;
  char buf_send[1500] = {0};
  char buf_recv[1500] = {0};
  int numbytes = 0;
  printf("local_ip: %s, local_port:%s\n", local_ip, local_port);

  if (server_init(local_port, &listen_fd) != HTTP_OK) {
    return HTTP_FAIL;
  }
  // printf("listening fd: %d\n", listen_fd);
  printf("server listening to client connection...\n");
  if (accept_client(listen_fd, &client_fd) != HTTP_OK) {
    return HTTP_FAIL;
  };

  if (!fork()) { // child process

    numbytes = readline(client_fd, buf_recv);
    printf("client message: %s\n", buf_recv);
    if (server_lookup_connect(target_site, REMOTE_PORT, &server_fd) !=
        HTTP_OK) {
      return HTTP_FAIL;
    };
    // copy http request line from client message
    // memcpy(buf_send, request_line, sizeof(request_line));
    memcpy(buf_send, buf_recv, sizeof(request_line));

    writen(server_fd, buf_send, sizeof(buf_send));
    readline(server_fd, buf_recv);
    printf("server return:\n");
    printf("%s", buf_recv);
    writen(client_fd, buf_recv, sizeof(buf_recv));
    printf("child process returns\n");
    exit(0);
  } else { // parent process
    printf("parent process returns\n");
    wait(NULL);
  }

  return HTTP_OK;
}

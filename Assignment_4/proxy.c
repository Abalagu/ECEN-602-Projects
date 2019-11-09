#include "headers.h"

http_err_t main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: ./proxy <ip to bind> <port to bind>\n");
    return HTTP_FAIL;
  }

  char request_line[] = "GET /get HTTP/1.1\n\
Host: httpbin.org\n\
User-Agent: PostmanRuntime/7.19.0\n\
Accept: */*\n\
Cache-Control: no-cache\n\
Postman-Token: ff6456d8-34c6-4c58-8e9c-5a45091c2f08,ec4aab86-2482-491a-816b-420dd3613cc5\n\
Host: httpbin.org\n\
Accept-Encoding: gzip, deflate\n\
Connection: keep-alive\n\
cache-control: no-cache\n\
";
  // printf("%s\n", request_line);
  char target_site[] = "httpbin.org";

  char *local_ip = argv[1]; // why specify ip other than localhost?
  char *local_port = argv[2];
  int listen_fd;
  char buf_send[1500] = {0};
  char buf_recv[1500] = {0};
  // struct sigaction sa;

  // reap_dead_processes
  // sa.sa_handler = sigchld_handler;
  // sigemptyset(&sa.sa_mask);
  // sa.sa_flags = SA_RESTART;
  // if (sigaction(SIGCHLD, &sa, NULL) == -1) {
  //   perror("sigaction");
  //   exit(1);
  // }

  server_init(local_port, &listen_fd);

  if (!fork()) { // child process
    recv(listen_fd, buf_recv, sizeof(buf_recv), 0);
    int sockfd = server_lookup_connect(target_site, REMOTE_PORT);
    memcpy(buf_send, target_site, sizeof(target_site));
    writen(sockfd, buf_send, sizeof(buf_send));
    readline(sockfd, buf_recv);
    printf("%s", buf_recv);
    printf("child process returns\n");
    exit(0);
  } else { // parent process
    // break;
    // continue;
    printf("parent process returns\n");
    wait(NULL);
  }

  return HTTP_OK;
}

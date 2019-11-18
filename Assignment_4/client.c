#include "headers.h"
#include "lib.h"
http_err_t open_file(FILE **fp, char *filename, char *mode) {
  *fp = fopen(filename, mode);
  if (*fp == NULL) {
    printf("file: %s. open failed.\n", filename);
    return HTTP_FAIL;
  } else {
    return HTTP_OK;
  }
}
size_t write_data_to_file(FILE **fd, char *buf, size_t _numbytes,
                          off_t offset) {
  size_t numbytes;
  numbytes = fwrite(buf + offset, 1, _numbytes, *fd);
  return numbytes;
}

char *get_filename(char *str) {
  /* get the first token */
  const char separator[] = "/";
  char *token, *tmp;
  token = strtok(str, separator);

  /* walk through other tokens */
  while (token != NULL) {
    tmp = token;
    token = strtok(NULL, separator);
  }
  return tmp;
}

http_err_t main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("usage: ./client <proxy address> <proxy port> <URL to rerieve>\n");
    return HTTP_FAIL;
  }
  http_err_t retval;

  // TODO:: validate args

  char *proxy_address = argv[1];
  char *proxy_port = argv[2];
  char *url;

  // this client supports all the possbile formats
  //  "www.cpluscplus.com";
  //  "www.cpluscplus.com/some";
  //  "https://www.cpluscplus.com";
  //  "https://www.cpluscplus.com/some";

  //  printf("address: %s, port: %s, target: %s\n", proxy_address, proxy_port,
  //  argv[3]);

  int _size = strlen(argv[3]);

  url = malloc(_size * sizeof(char));
  memcpy(url, argv[3], _size);

  int numbytes = 0;
  printf("address: %s, port: %s, target: %s\n", proxy_address, proxy_port, url);

  int sockfd = 0;

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

  char *filename = strdup(path);

  printf("request: %s\n", request);
  memcpy(buf_send, request, sizeof(request));
  printf("path: %s\n", path);

  if (server_lookup_connect(proxy_address, proxy_port, &sockfd) != HTTP_OK) {
    return HTTP_FAIL;
  }
  numbytes = written(sockfd, request, strlen(request) + 1);
  printf("sent request to server\n");
  filename = get_filename(filename);
  printf("filename: %s\n", filename);
  FILE *fp;
  open_file(&fp, filename, "a");

  // first recv treatment
  numbytes = readline(sockfd, buf_recv, MAX_DATA_SIZE);
  http_info_t *http_info = calloc(1, sizeof(http_info_t));
  parse_response(buf_recv, http_info);
  int content_length = atoi(http_info->content_length);
  printf("content length: %d\n", content_length);

  char *entity_body_head = strstr(buf_recv, "\r\n\r\n");
  off_t offset = entity_body_head - buf_recv + 4;
  printf("offset: %ld\n", offset);
  numbytes = write_data_to_file(&fp, buf_recv + offset, numbytes - offset, 0);
  int count = numbytes;

  while (1) {
    numbytes = readline(sockfd, buf_recv, MAX_DATA_SIZE);
    if (numbytes == 0) {
      fclose(fp);
      break;
    } else {
      offset = 0;
      if (count + numbytes > content_length) {
        offset = count + numbytes - content_length;
        write_data_to_file(&fp, buf_recv, numbytes - offset, 0);
        close(fp);
        exit(0);
      } else {
        write_data_to_file(&fp, buf_recv, numbytes, 0);
        count += numbytes;
      }
      printf("written %ld bytes to file..\n", numbytes - offset);
    }
  }
  // printf("\n\nResponse:\n%s\n", buf_recv);
}

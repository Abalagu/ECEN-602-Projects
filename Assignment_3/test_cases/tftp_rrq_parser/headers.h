#ifndef HEADERS_H_
#define HEADERS_H_

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // memset() is missing in strings.h
#include <strings.h>  // include strings.h for strncasecmp()
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "config.h"

#endif
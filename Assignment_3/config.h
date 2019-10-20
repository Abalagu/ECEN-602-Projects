#ifndef CONFIG_H_ 
#define CONFIG_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEBUG 0

#define MAXBUFLEN 512 
#define MAX_FILE_NAME 255
#define DATA_PACKET_LEN 516; // opcode + #block + payload = 2 + 2 + 512 = 516

#endif
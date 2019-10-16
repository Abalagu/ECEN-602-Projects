#include "config.h"
#include "lib.h"

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int init() {
	int sockfd;
  struct  addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
 		return 1;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
    	p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }

   	if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    return 2;
  }

  freeaddrinfo(servinfo);

  return sockfd;
    
}

int main() {
  char s[INET6_ADDRSTRLEN];
  char buf[MAXBUFLEN];
	char *rbuf;
  int numbytes;
	char filename[MAXBUFLEN];
	char mode[8]; // netascii, octet or mail
  socklen_t addr_len;
	opcode_t opcode;
  struct sockaddr_storage their_addr;
  int sockfd;

  sockfd = init();

  printf("listener: waiting to recvfrom \n");

  addr_len =  sizeof their_addr;

  if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
      (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      perror("recvform");
      exit(1);
  }

  printf("listner: got packet from %s \n",
      inet_ntop(their_addr.ss_family, get_in_addr((struct  sockaddr *)&their_addr),
      s, sizeof s));

	if (DEBUG) {
  	printf("[Packet size] %d\n", numbytes);
	}

  buf[numbytes] = '\0';
	// for (int i = 0; i < numbytes; i++) {
  //     printf("%d", buf[i]);
  // } 
  printf("listener: packet contains : \n" );
	// memcpy(tftp_header, buf, 16);
  for (int i = 0; i < numbytes; i++) {
			printf("%d:", i);
      printf("%d %c\n",buf[i], buf[i]);
			// memcpy(rbuf * i + 16, buf, )
  } 

	tftp_header_t *tftp_header= (tftp_header_t *) buf;
	opcode = (tftp_header->opcode) >> 8;
	// this will give me two byte opcode and the rest of the 
	// buffer in the finename array.
	// then read that filename array into a string untill you encounter a space,
	// then read the mode and move
	// opcode = 1;
	if (DEBUG)	
		printf("[Opcode] %d\n", opcode);

	// for (int i = 0; i < numbytes; i++) {
  //     printf("%d \n",tftp_header->buf_rem.filename[i]);
  // } 
	char *pos_ptr = strchr(tftp_header->trail_buf, 0);
	int pos = (pos_ptr == NULL ? -1 : pos_ptr - tftp_header->trail_buf);

	if (pos < 0) {
		perror("filename not valid\n");
	}

	printf("pos:%d\n", pos);
  memcpy(filename, tftp_header->trail_buf, pos);
	
	if (DEBUG) {
		printf("[Passed filename] ");
		for (int i = 0; i < pos; i++) {
      printf("%c",filename[i]);
		}
		printf("\n");
	}

	printf("[trailing header] ");
	for (int i = 0; i < numbytes; i++) {
 		printf("%d \n",tftp_header->trail_buf[i]);
	}	
	printf("\n");

	pos_ptr = strrchr(tftp_header->trail_buf, 0);
	int last_pos =  (pos_ptr == NULL ? -1 : pos_ptr - tftp_header->trail_buf);
	
	if (last_pos < 0) {
		perror("filename not valid\n");
	}
	printf("last_pos:%d\n", last_pos);
	memcpy(mode, &(tftp_header->trail_buf[pos+1]), sizeof mode);

	if (DEBUG) {
		printf("[Mode] ");
		for (int i = 0; i < 8; i++) {
      printf("%c",mode[i]);
		}
		printf("\n");
	}




	switch (opcode) {
	case RRQ: 
		printf("[Read request] \n");
		break;
	
	case WRQ:
		printf("[Write request] \n");
		break;

	case DATA:
		break;

	case ACK:
		break;

	case ERROR:
		break;

	default:
		break;
	}


  close(sockfd);

  return 0;

}
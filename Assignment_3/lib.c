#include "lib.h"

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int init(char *port) {
	int sockfd, rv, numbytes;
	struct  addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
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

void print_hex(void *array, size_t len) {
  char *parray = array;
  for (int i = 0; i < len; i++) {
    printf(" %02x", parray[i]);
  }
  printf("\n");
}



void rrq_handler(char *filename){
	char buf_send[516] ={0};
	bool next_block = 1;
	printf("filename: %s\n", filename);
	tftp_data_t data_packet = {0};
	data_packet.opcode = 1;
	data_packet.block = 0;
	memset(data_packet.payload, 20, 512);
	memcpy(buf_send, &data_packet, sizeof(tftp_data_t));	
	print_hex(buf_send, sizeof(tftp_data_t));
}

opcode_t parse_header(char buf[MAXBUFLEN], char filename[MAXBUFLEN], char mode[8]) {
	/* parse the buffer and fill in filename and mode and return the opcode */	

	tftp_header_t *tftp_header= (tftp_header_t *) buf;
	/* extract opcode */
	opcode_t opcode = (tftp_header->opcode) >> 8; 									

	if (DEBUG) {	
		printf("[Opcode] %d\n", opcode);
	} 
	
	/* trailing buffer now has filename and the mode. According to RFC, the file
		name is followed by a 0. Locate that 0 */
	char *pos_ptr = strchr(tftp_header->trail_buf, 0);
	/* in case pos reaches the end of the file, it's a problem */
	int pos = (pos_ptr == NULL ? -1 : pos_ptr - tftp_header->trail_buf);

	if (pos < 0) {
		perror("filename not valid\n");
		//TODO:: send an error.
	}

	/* extract filename from the trail buffer */
	memcpy(filename, tftp_header->trail_buf, pos);
	
	if (DEBUG) {
		printf("[Passed filename] ");
		for (int i = 0; i < pos; i++) {
			printf("%c",filename[i]);
		}
		printf("\n");
	}

	/* rest of the buffer has mode */	
	memcpy(mode, &(tftp_header->trail_buf[pos+1]), 8*sizeof(char));

	if (DEBUG) {
		printf("[Mode] ");
		for (int i = 0; i < 8; i++) {
			printf("%c",mode[i]);
		}
		printf("\n");
	}

	return opcode;
}
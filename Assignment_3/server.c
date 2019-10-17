#include "config.h"
#include "lib.h"


int main(int argc, char* argv[]) {
	
	//TODO:: validate passed args

  char s[INET6_ADDRSTRLEN], buf[MAXBUFLEN], filename[MAXBUFLEN];
  int sockfd, numbytes;
	char mode[8]; // netascii, octet or mail, should never exceed 8 bytes

	opcode_t opcode;

  socklen_t addr_len;
  struct sockaddr_storage their_addr;
  addr_len =  sizeof their_addr;

	/* initialize the server */
  sockfd = init();
  printf("server: waiting to recvfrom \n");

  if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
    (struct sockaddr *)&their_addr, &addr_len)) == -1) {
    perror("recvform");
    exit(1);
  }

  printf("server: got packet from %s \n",
    inet_ntop(their_addr.ss_family, get_in_addr((struct  sockaddr *)&their_addr),
    s, sizeof s));

  // buf[numbytes] = '\0';
	if (DEBUG) {
  	printf("[Packet size] %d\n", numbytes);
  	printf("[Packet content] : \n" );
  	for (int i = 0; i < numbytes; i++) {
			printf(" [%d]:", i);
      printf(" %d %c\n",buf[i], buf[i]);
  	} 
	}

	tftp_header_t *tftp_header= (tftp_header_t *) buf;
	/* extract opcode */
	opcode = (tftp_header->opcode) >> 8; 									

	if (DEBUG) {	
		printf("[Opcode] %d\n", opcode);
	} 
	
	/* trailing buffer now has filename and the mode. According to RFC, the file
		name is followed by a 0. Locate that 0 */
	char *pos_ptr = strchr(tftp_header->trail_buf, 0);
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
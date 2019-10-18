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

	/* parse the received header */
	opcode = parse_header(buf, filename, mode);

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
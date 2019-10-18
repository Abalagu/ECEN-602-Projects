TFTP Server

* Server

## TFTP Server Expect Message Sequence From Client:

### Client send "get xx" to Server

	1. RRQ
	2. ACK
	3. ACK

	...
	m. ERROR (child process termination)
	...
	n. ACK (final)

	

### Client send "put xx" to Server

	1. WRQ
	2. DATA
	3. DATA

	...
	m. ERROR (child process termination)
	...
	n. DATA (final, len < 512)

### Server Architecture
``` c
void rrq_handler(char *filename){
	bool next_block = 1; // handle timeout resend
	// terminate when having 10 continuous timeout
	// reset counter when a valid ACK packet is received
	int timeout_counter = 0; 
	// send DATA, expect ACK or ERR
	fd_child = socket();

	while (1) { // read till EOF
		if (next_block) { // read next block of file
			buf_send = read(local_file);
		}
		sendto(buf_send);

		if (select(fd_child, tv=1.0s)) { // something to read
			buf_read = recvfrom();
			type = parse_header(buf_read);
			if (type == ERR) {
				parse_error(buf_read); // print error message
				return 1; // child process exit
			}
			if (type == ACK) {
				block_number = parse_ack(buf_read);
				// validate not an out of order or duplicate ACK
				// set next_block = true if pass validation
				next_block = validate_ack(block_number);
				timeout_counter = 0;
			}
			// discard other types of packets
		} else { // timeout triggered, ACK didn't arrive
			next_block = 0; // won't read next block
			timeout_counter += 1;
			// terminate process when having 10 consecutive timeout
			// indicating that remote disconnected
			if (timeout_counter >=10) { 
				return 1;
			}
		}
	}
}

void wrq_handler(){

}

int main() {

	init();
	// if connection
	while(1){ // loop on listening
		buf, remote_addr = recvfrom();
		// fork after parent process recv packets
		if (!fork()){ //child process
			//check packet type then call specific parser
			type = parse_header(buf);
			if (type == RRQ) { // server RRQ routine
				filename = parse_rrq(buf);
				rrq_handler(filename, remote_addr);
			}
			if (type == put) { // server WRQ
				// similar logic as RRQ
				filename = parse_wrq(buf);
				wrq_handler(filename);
			}
		} else{ //parent process
			// reap child process
			wait(NULL);
		}
	}
}
```


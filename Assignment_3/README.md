## Program Architecture

### RRQ

* Server is able to handle multiple client RRQ requests and transmit simutaneously, by creating a child process for each request.

* Server handles various packet types with constructing and parsing functionalities under the `lib.c` file.

* Block number wrap around is implemented by utilizing overflow of `uint16_t`.

* Error packets are replied to the client when the requested file name doesn't exist under the tftp server folder.

* Packet timeout is implemented by using timer tv in the select call of the child process.  When 10 consecutive timeout happens, the server replies with an ERROR packet, and terminates the corresponding child process.  

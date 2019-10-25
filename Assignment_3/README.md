# TFTP Server

## Role of Team Members

* Luming Xu
  - RRQ
  - Binary file test cases

* Akhilesh Rawat
  - WRQ
  - Ascii File transfer for WRQ and RRQ
  - Ascii file test cases

## Bonus Points

We implement the WRQ feature in the server. The client can write both binary and ascii files to the server and retrieve them later. The files maintain their format and are identical to the line endings as is desired. We test the following features:
  - transfer binary files of sizes 2047B, 2048B and 3MB from client to the server and retrieve them later
  - transfer ascii files with different line endings from client to the server and retrieve them later
  - timeout if the client disconnects while transfer
  - simultaneous writes to the server.
  - error if the file already exists at the server

## Program Architecture

* On a RRQ or WRQ request from a client, the server creats a child process for each request. This way the server is able to handle multiple client RRQ  and WRQ requests and transmit simutaneously. 

* `lib.c` provides with construction and parse APIs for various packet types.
* Various server parameters can be configured with `config.h`.

* Block number wrap around is implemented by utilizing overflow of `uint16_t`.

* Error packets are replied to the client when the requested file name doesn't exist under the tftp server folder or if the client attempts to write a file which already exists.

* Packet timeout is implemented by using timer tv in the select call of the child process.  When 10 consecutive timeout happens, the server replies with an ERROR packet, and terminates the corresponding child process.  

* For ascii files, the server recognizes the mode and reads and writes the files accordingly by updating line endings to preserve the file format. The `read_block` and `write_data_to_file` implement the relevant logic.


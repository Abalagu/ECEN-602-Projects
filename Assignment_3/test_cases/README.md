# README

## Notes on test cases

### udp_basic

* execute `./run_server.sh` and `./run_client.sh` 
* client sends a string to server via UDP, server prints out, and closes connection.

### tftp_parse

* execute `./run_server.sh` and `./run_client.sh` 
* this time uses standard TFTP client, which connects to manually set server port `4950` , then type `get foo` in `tftp>` .
* server receives an array of bytes and prints in hex format.

## Implementation Notes

### 1.enable typedef on `addrinfo` 

* modified file `/usr/include/features.h` , add `#define _XOPEN_SOURCE 700` to the file, which represents using 7th edition.
* after defining the macro, VS Code won't complain about `incomplete type` on addrinfo.


# TCP Simple Broadcast Chat Server and Client

## Code Style

* Using snake_case 
* indent with tabsize: 4

## Role of Team Members

* Luming Xu
    - client code
    - message parser and constructor
    - test cases

* Akhilesh Rawat
    - server code
    - broadcasting
    - Makefile

## Program Execution:

1. run ./run_server.sh
2. run ./run_client.sh

## Program Architecture:

### Server

* server monitors multiple file descriptor(fd) using select to accept new client, while at the same time read and forward message to other clients.
* server associates each fd with a client's username, and controls maximum number of clients in the chat.

### Client

* client is using select to monitor FWD message and STDIN at the same time.
* message is packaged in fixed length struct, supports multiple attributes.

## Test Cases

### Case 1

#### normal operation of the chat with three clients connected

### Case 2

#### server rejects a client with a duplicate username

### Case 3

#### server allows a previously used username to be reused

### Case 4

#### server rejects the client because it exceeds the maximum number 

### Case 5

#### IPv4 and IPv6 support

### Case 6

#### bonus feature 1

### Case 7

### bonus feature 2

## Source Code

### config.h

``` c
```

### Makefile

``` Makefile
```

### run_client.sh

``` bash
```

### run_server.sh

``` bash
```

### server.c

``` c
```

### server.h

``` c
```

### server_lib.c

``` c
```

### server_lib.h

``` c
```

### client.c

``` c
```

### client.h

``` c
```

### client_lib.c

``` c
```

### client_lib.h

``` c
```

### common_lib.c

``` c
```

### common_lib.h

``` c
```


# Program Architecture

## Implementation Notes

### HTTP Client:

* Client closes connection after retrieving the document
* Client should write the retrieved document to the local disk
* Client should be able to tell from the status code if a document is returned

### HTTP Proxy Server:

* Limit simultaneous connection of clients to maximum of 3
* Use a while loop on select call in a single process to handle multiple clients.
* Transition stages
    1. accept client connection
    2. receive client get request

        (parse, LRU cache operation)

    3. connect to http server
    4. send http request to server
    5. receive http response from server
    6. send http response to client

* maintain file descriptors with below functionalities:
  + listening to client connection
  + handle communication with client
  + handle communication with remote http server

* close connection at receiving EOF

## Special Scenarios

* If speed or exception is not an issue, I'd love to make all RX TX blocking.

### Scenario 1: Single process, multiple outstanding send, with speed variation

* Suppose three clients, namely A, B, and C, connects to the proxy server. Additionally, suppose the requested documents of the three are all present in the LRU cache. Now A is requesting a huge document that needs 1 hour transmission time, and/or is under severe congestion. Meanwhile B and C request only a tiny document that transmits instantly. If B and C subsequently request more documents, file A would fall into the least used file, and liable to be evicted from the cache, which might break its future transmission.

#### Design Goal:

* to avoid interrupting transmission of any document with speed variation

#### Proposed solution: 

* Add an IN_USE flag to the structure holding the document. If it's not IN_USE, evict the document with memory release. Otherwise, leave memory release to the undergoing process that's using it.

  + The LRU document can be evicted without causing any problems, even if its in use, as cache only holds the address of the struct that stores the actual document, which can be associated with the socket fd that's using it.

* restrict simultaneous connected clients to be less than the cache slot size, thus clients cannot possess all cache slots at any given time.

### Scenario 2: Single process, multiple outstanding cache write, with speed variation

* Suppose three clients, namely A, B, and C, connects to the proxy server. Additionally, assume that files are all not present in the cache. Now proxy server is downloading document to cache for client A, which takes a while. Meanwhile, if B and C are requesting the same document(same url) from the cache, which is not fully prepared, one faces the decision of either waiting for A to finish, or start anew.

#### Proposed solution:

* The document should not be stored in the cache if it's partial, i.e., under transmission. It should first be fully fetched to the memory.

* One is safe to start its own caching even if there exists one in progress, either stuck, slow, or for whatever reason .

## LRU Cache Management

### Document storage process

1. allocate memory for a node holding the document
2. finish document retrival from the server
3. add the node to the cached list
* No partial document in the cache.

### Document reorder process

* Suppose it takes several reads to finish transfering a document to a client
1. place the document to the front at the first read block
2. do not reorder the document for any subsequent read blocks
3. reorder the document to the front if it's requested from another client

### General Routine (Optimized)

1. Init: fill all cache slots with dummy nodes

2. Service:
* if cache hit:
  + if cache fresh: reorder document
  + if cache stale: 
    - if conditional get returns 304 Not Modified: reorder document
    - if conditional get returns 200 OK: refresh the document
    - if conditional get returns others: leave it for now

* if cache miss:

	1. evict oldest
    - if IN_USE: do not free memory

    - if not IN_USE: free the node memory

	2. add new document

## Pseudocode

### HTTP Client

``` c
int main(proxy_addres, proxy_port, url){
    proxy_fd = server_connect(proxy_address, proxy_port);
    http_request = make_http_request(url);
    send(proxy_fd, http_request);
    http_response = recv(proxy_fd);
    write(http_response);
    close(proxy_fd);
}
```

### HTTP Proxy Server

``` c
bool cache_hit(struct request, char *document){
	if (request.url in cache nodes){
		*document = node.document;
		return true;
	} else{
		return false;
	}
}

int main(ip, port){
    listen_fd = init_server(ip, port);
    read_fds = [];
    write_fds = [];
	client_fds = [];
	server_fds = [];

    while(1){
        if(select(read_fds, write_fds)<=0){
            continue;
        }
        if(ISSET(listen_fd)){
            client_fd = accept_client(listen_fd);
            read_fds = [read_fds, client_fd];
        }
        for(client_fd in read_fds){
            if (ISSET(client_fd)){
				// GET request is not lengthy, to make things simpler, assume to receive it in one call
                buf_recv = recv(client_fd); 
                struct request = parse_request(buf_recv);
				retval = cache_hit(request, document);

                if(retval == STATUS.FRESH){
					http_response = make_http_response(document);
					read_fds.remove(client_fd);
					write_fds.append(client_fd);
                }else if(retval == STATUS.STALE or STATUS.MISS) {
                    server_fd = server_connect(request.host);
                    http_request = make_http_request(struct request, RETVAL);
					write_fds.append(server_fd);
					send(server_fd, http_request);
					read_fds.append(server_fd);
                }
            }
        }
    }
}

```

``` 
client_fd = accept(); // on connection

read_fds.append(client_fd); // on receiving request
read_fds.remove(client_fd); // on request fully received

write_fds.append(server_fd); // on GET / CONDITIONAL GET
write_fds.remove(server_fd); // on fully received server response

write_fds.append(client_fd); // on fully received server response
write_fds.remove(client_fd); // on fully transmitted server response
```


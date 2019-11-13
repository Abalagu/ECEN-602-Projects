# Program Architecture
## Client Architecture
```c
int main(){
    argv[1] = url;
    host_name, document_name = parse_url(url);
    proxy_fd = connect();
    // embed host name in the header, document name in get line
    buf_send = make_get_request(host_name, document_name);
    send(proxy_fd, buf_send);
    recv(proxy_fd, buf_recv); // should read til EOF
    fwrite(buf_recv);
    close(proxy_fd);
    return 0;
}

```

## Proxy Server Architecture
### Implementation Note
1. Spawn a child process only during file streaming from proxy server to a client
2. Maintain a LRU cache only in the parent proxy server process
3. Proxy server retrieve a document to local memory, then fork a process to stream to client.  All LRU cache modification happens before fork.  Terminate the process after streaming.  

#### Side Notes
* If spawn a child process starting from connection, and implement LRU cache in the shared memory across all child processes, both read and write from LRU cache will cause memory content to change, thus simultaneous read is impossible, one has to make read from cache ordered.  
* When is LRU cache changed?
    * when cache is full
        * when cache miss 
            1. remove least used document
            2. add new document to front
        * when cache hit
            1. reorder document
    * when cache is not full
        * when cache miss
            1. add new document to front
        * when cache hit
            1. reorder document
* Optimized
    * if cache hit:
        reorder document
    * if cache miss and cache full:
        remove LRU, add new
    * if cache miss and cache not full:
        add new


### Proxy Server State Transition
1. accept client connection
2. receive client get request
3. connect to http server
4. send http request to server
5. receive http response from server
6. send http response to client


At step 2, check if client request hits cache.
* return True if (cache hit and document fresh)
* return False if (cache hit but document stale, or cache miss)
```
if (cache_hit(host_name, file_name)){
    if(is_document_fresh(host_name))
} else{ // cache not hit

}
```
If cache hit, then 
If cache miss, then execute step 3-6
```c
int main(){
    listen_fd = server_init();
    read_fds = []
    write_fds = []
    while(1){
        if(select()<=0){
            continue;
        }
        if(isset(listen_fd)){
            client_fd = accept_client(listen_fd);

        }
    }
    
}
```

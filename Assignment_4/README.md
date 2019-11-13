# HTTP Proxy

* cache
* select
* post
* get

## HTTP Proxy Server

``` 
  if listen() == 1
    // request in
    parse_request();
    if check_cached_data() == 1
      respond_to_client();
    else
      proxy_to_destination() GET // request
      cache_response();
        parse_response_from_destination();
        store_in_cache();
      respond_to_client(); POST // response
```

## HTTP command line client

  GET // request

## proxy_cache

  + LRU

	-	cache the entire response

  if expires and last_modified missing:

    do_not_cache();
    

## status codes

## parsing

* for the first line:
  + retrieve method, protocol, status code, path depending on the type of REQUEST, 

* then pass the rest of the buffer for parsing:
* pass the char buffer, can be anything with the following format

    key:value \n

  and along with it pass the struct you want to fill in; 

* api: `parse(buf, struct response/request)` 

* default the struct fields to zero before passing.

* go through the buffer, using line breaks.

*  After every line break, the next line, until ':', that gives the key, 

  after ':', it is the value.

* loop untill all the char buffer is exhausted, and fill the passed struct:

    struct.key = value

## HTTP Client Architecture

* maintain a file descriptors with proxy server

``` c
server_lookup_connect();
# send url to proxy server
writen();
# receive url from proxy server
readline();
```

## HTTP Proxy Server Architecture

### difficulties

1. read stream from http server, payload could exceed buffer
2. dynamically allocate cache memory for received document
3. implement shared memory for LRU Cache
4. multithreading for each child process

### implementation notes
* should use multi-threading, as test case 7 is about simultaneous connection
* probably better to use only single thread in the proxy server, as cache management across child threads is difficult, need to use shared memory
* maintain file descriptors with below functionalities:
  + listening to client connection
  + handle communication with client
  + handle communication with remote http server
* proxy server and http server closes connection at EOF

In the point of view of a proxy server, it takes several stages to complete a cycle:

1. client connection established (listen_fd -> new client_fd)
2. client http request received (client_fd, read_fds)
3. server connection established (new server_fd)
4. http request sent to server (server_fd, write_fds)
5. http response received from server (server_fd, read_fds)
6. http response written(relayed) to client (client_fd, write_fds)

If requested document is not in the cache, then proxy server goes through all six stages.

If requested document is in the cache and is fresh, then return the cache

### cache data structure

``` c
typedef struct cache {
  document[];
  
}
```

``` c

http_err_t prepare_document(char *http_response){
  if(not in_cache(url)){
    http_request = make_request(document_name); // construct get request
    server_fd = server_lookup_connect(); // establish connection to http server
    send(server_fd, http_request); // send http request
    *http_response = read(server_fd); // get http response
    close(server_fd);
    return OK
  }
  if(is_fresh(host_name, document_name)){
    return OK
  }
  
}

int main(){
  // 
  
  listen_fd = server_init(port); // persistent open until end of execution
  // only maintain server_fd in the list, as listen_fd requieres different action
  // need a data structure to maintain add and delete of fds
  read_fds = [];
  write_fds = [];
  while(1){ // proxy service
    if(select(read_fds, write_fds)){ // if some fd is set
      if (isset(listen_fd)){ // new client connection
        client_fd = accept_client(listen_fd);
        // only monitor read from client after connection
        read_fds = [read_fds, client_fd];
      }
    }
  }
  
  for (read_fd in read_fds){
    if(isset(read_fd)){

    }
  }
  client_fd = accept_client(); // close after transmission
  url = read(client_fd); // get http request from client
  host_name, document_name = parse(url);
  // cache processing
  prepare_document(); // whether in cache, or retrieve from server
  write(client_fd, http_response);
  close(client_fd);

  while(1){ // infinite loop on listening to new connection
    read_fds = [listeninf_fd, server_fd]
    write_fds = [client_fd]
    select()
    client_fd = accept_client(listen_fd); // blocking accept
    
    
  }

}

```


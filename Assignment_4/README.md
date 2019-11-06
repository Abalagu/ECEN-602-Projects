# HTTP Proxy
- cache
- select
- post
- get


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
  - LRU
	-	cache the entire response
  if expires and last_modified missing:
    do_not_cache();
    
## status codes

## parsing

- for the first line:
	- retrieve method, protocol, status code, path depending on the type of REQUEST,

- then pass the rest of the buffer for parsing:
- pass the char buffer, can be anything with the following format
    key:value \n

  and along with it pass the struct you want to fill in;
- api: `parse(buf, struct response/request)`

- default the struct fields to zero before passing.

- go through the buffer, using line breaks. 

-  After every line break, the next line, until ':', that gives the key,
  after ':', it is the value.

- loop untill all the char buffer is exhausted, and fill the passed struct:
    struct.key = value
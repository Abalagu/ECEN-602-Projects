C = gcc
# DEPS = headers.h 

TARGETS = server client 

all: $(TARGETS)

server: server.o server_lib.o common_lib.o
	$(C) -o server server.o server_lib.o common_lib.o

server.o : server.h

server_lib.o : server_lib.h

client : client.o client_lib.o common_lib.o
	$(C) -o client client.o client_lib.o common_lib.o

client.o : client.h

client_lib.o : client_lib.h

common_lib.o : common_lib.h

clean:
	rm -f *.o server client 
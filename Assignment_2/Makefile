C = gcc
# DEPS = headers.h 

TARGETS = server client 

all: $(TARGETS)

server: server.o server_lib.h common_lib.h
	$(C) -o server server.o

client : client.o client_lib.o common_lib.o
	$(C) -o client client.o client_lib.o common_lib.o

client.o : client.h

client_lib.o : client_lib.h

common_lib.o : common_lib.h

clean:
	rm -f *.o server client 
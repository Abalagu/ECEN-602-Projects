C = gcc
DEPS = headers.h 

TARGETS = server client 

all: $(TARGETS)

server: server.o $(DEPS)
	$(C) -o server server.o
client: client.o $(DEPS)
	$(C) -o client client.o
	
clean:
	rm -f *.o *~ server client 
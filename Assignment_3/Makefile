C = gcc
DEPS = config.h lib.h 

TARGETS = server 

all: $(TARGETS)

server: server.o lib.o $(DEPS) 
	$(C) -o server server.o lib.o 

clean:
	rm -f *.o server  

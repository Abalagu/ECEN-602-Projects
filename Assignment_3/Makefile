
C = gcc
DEPS = config.h lib.h 

TARGETS = server 

all: $(TARGETS)

server: server.o $(DEPS) 
	$(C) -o server server.o 

clean:
	rm -f *.o server 

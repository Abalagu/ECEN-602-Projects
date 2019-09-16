C = gcc
DEPS = headers.h 

TARGETS = echos echo

all: $(TARGETS)

echos : server.o $(DEPS)
	$(C) -o echos server.o
echo : client.o $(DEPS)
	$(C) -o echo client.o
	
clean :
	rm -f *.o *~ echos echo
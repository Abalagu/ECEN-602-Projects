all : client proxy

client : client.o lib.o
	cc -o client client.o lib.o

proxy : proxy.o lib.o
	cc -o proxy proxy.o lib.o

.PHONY : clean
clean :
	rm *.o client proxy
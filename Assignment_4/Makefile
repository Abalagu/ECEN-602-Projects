all : client proxy

client : client.o lib.o
	cc -o client client.o lib.o

proxy : proxy.o lib.o
	cc -o proxy proxy.o lib.o

proxy_debug: lib.o
	gcc -g proxy.c -o proxy lib.o

.PHONY : clean
clean :
	rm *.o client proxy

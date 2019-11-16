all : client proxy

client : client.o lib.o
	cc -o client client.o lib.o

proxy : proxy.o lib.o
	cc -o proxy proxy.o lib.o


# lib_debug : lib.o
# 	gcc -g -c lib.c lib.o

proxy_debug: 
	gcc -g -c lib.c
	gcc -g proxy.c -o proxy lib.o

.PHONY : clean
clean :
	rm *.o client proxy

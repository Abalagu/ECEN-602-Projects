#include "headers.h"

int main (int argc, char *argv[])
{
    if (argc != 4) {       
        fprintf(stderr, "Usage: server server_ip server_port max_clients\n");
        exit(1);
    }
}
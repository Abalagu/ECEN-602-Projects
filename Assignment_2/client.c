#include "headers.h"
#include "config.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: client username server_ip server_port\n");
        exit(1);
    }
}

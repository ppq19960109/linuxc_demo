#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("main start\n");
    if (argc > 1)
    {
        client_main(argv[1]);
    }
    else
    {
        server_main();
    }

    return 0;
}

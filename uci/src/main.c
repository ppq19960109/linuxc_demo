#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "uci_wifi.h"
#include "wifi_server.h"

// #include "uci.h"
// int uci_test()
// {
//     struct uci_context *c;
//     struct uci_ptr p;
//     char *a = strdup("wireless.@wifi-iface[1].network");

//     c = uci_alloc_context();
//     if (UCI_OK != uci_lookup_ptr(c, &p, a, true))
//     {
//         uci_perror(c, "no found!\n");
//         return -1;
//     }

//     printf("%s\n", p.o->v.string);
//     uci_free_context(c);
//     free(a);
//     return 0;
// }

int main(int argc, char *argv[])
{
    printf("main start\n");
    // uci_wifi_set(argv[1], argv[2], NULL);
    
    thread_wifi_config();
    wifi_client_open();
    while (1)
    {
        sleep(1);
    }
    return 0;
}

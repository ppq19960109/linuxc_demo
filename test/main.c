#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <stdarg.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

int main(int agrc, char *agrv[])
{
    printf("main start \n");
    if (UINT_MAX + 2 > UINT_MAX)
    {
        printf("UINT_MAX+2>UINT_MAX\n");
    }
    else
    {
        printf("UINT_MAX+2<UINT_MAX\n");
    }
    printf("UINT_MAX+2=%u\n", (unsigned int)(UINT_MAX + 2));

    unsigned int val = 1;
    if (val - 2 < val)
    {
        printf("val - 2 < val\n");
    }
    else
    {
        printf("val - 2 > val\n");
    }
    printf("val - 2=%u\n", (unsigned int)(val - 2));
    return 0;
}

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
    // i2c_read_reg();
    i2c_only_write();

    i2c_only_read();
    return 0;
}

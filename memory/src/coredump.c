#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dumpCrash()
{
    // char *ptr = "test";
    // free(ptr);

    unsigned char *ptr = 0x00;
    *ptr = 0x00;
}
int main3()
{
    dumpCrash();
    return 0;
}
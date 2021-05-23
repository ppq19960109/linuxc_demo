#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dumpCrash()
{
    char *ptr = "test";
    free(ptr);
}
int main()
{
    dumpCrash();
    return 0;
}
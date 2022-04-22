#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "link_solo.h"

int main(int argc, char *argv[])
{
    printf("main start\n");
    // link_main("a1YTZpQDGwn", "oE99dmyBcH5RAWE3", "54ef337f47d3", "e6ac9ccdd1f1d56509ca2ce28d11d175", "1.0.0");//e6ac9ccdd1f1d56509ca2ce28d11d175
    link_main("a1YTZpQDGwn", "oE99dmyBcH5RAWE3", "54ef337f47d3", "e6ac9ccdd1f1d56509ca2ce28d11d175", "1.0.0");
    return 0;
}
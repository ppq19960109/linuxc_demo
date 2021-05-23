#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcheck.h>

void mcheck_func_cb(enum mcheck_status mc_status)
{
    printf("\n[%s:%d] mcheck_status=%d\n", __FILE__, __LINE__, mc_status);
}

int main1(int argc, char *argv[])
{
    // int ret = mcheck(mcheck_func_cb);
    // if (0 != ret)
    // {
    //     printf("[%s:%d] mcheck() failed\n", __FILE__, __LINE__);
    //     return -1;
    // }

    const char *src = "mcheck test";
    char *ptr = (char *)malloc(sizeof(char) * 32);
    memcpy(ptr - 1, src, strlen(src)); // 这里在堆内存块头部越界
    ptr[strlen(src) - 1] = '\0';
    printf("*ptr=%s\n", ptr);

    free(ptr);
    ptr = NULL;
    printf("process end\n");
    while(1);
    return 0;
}
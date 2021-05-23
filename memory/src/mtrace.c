#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcheck.h>
//export MALLOC_TRACE=./memleak.log
int main2()
{
    // setenv("MALLOC_TRACE", "./memleak.log", 1); /* 环境变量MALLOC_TRACE为记录文件路径,存放内存分配信息 */
    mtrace();

    char *text = (char *)malloc(sizeof(char) * 128);
    memset(text, 0, 100);
    memcpy(text, "hello,world!", 8);

    // printf("%s\n", text);
    // free(text);
    // int *p = (int *)malloc(2 * sizeof(int));
    muntrace();
    // unsetenv("MALLOC_TRACE");
    while(1);
    muntrace();
    return 0;
}

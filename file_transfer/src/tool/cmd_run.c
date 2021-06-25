#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int systemRun(const char *cmdline)
{
    printf("systemRun cmdline = %s\n", cmdline);

    int ret = system(cmdline);
    if (ret < 0)
    {
        printf("console_run cmdline failed: %s\n", cmdline);
    }
    return ret;
}

int popenRun(const char *cmdline, const char *mode, char *buf, int bufSize)
{
    printf("popenRun cmdline = %s\n", cmdline);
    FILE *pFile = popen(cmdline, mode);
    if (pFile == NULL)
    {
        return -1;
    }
    char *str = fgets(buf, bufSize, pFile);
    if (str == NULL)
    {
        pclose(pFile);
        return -1;
    }
    pclose(pFile);
    return 0;
}

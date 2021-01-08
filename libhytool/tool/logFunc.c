#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void logPrintf(char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void logPrintfHex(char *title,unsigned char *data, unsigned int len)
{
    printf("%s", title);
    for (int i = 0; i < len; ++i)
    {
        if (i % 16 == 0)
            printf("\n");
        printf("0x%x ", data[i]);
    }
    printf("\n");
}

const int consoleRun(const char *cmdline)
{
    printf("cmdline = %s\n", cmdline);
    int ret = -1;
    ret = system(cmdline);
    if (ret < 0)
    {
        printf("console_run cmdline failed: %s\n", cmdline);
    }
    return ret;
}

int popenRun(const char *cmd, const char *mode, char *buf, int bufSize)
{
    FILE *pFile = popen(cmd, mode);
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

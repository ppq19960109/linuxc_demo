#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"


void log_printf(char* format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

const bool console_run(const char *cmdline)
{
    printf("cmdline = %s\n", cmdline);
    int ret;
    ret = system(cmdline);
    if (ret < 0) {
        printf("Running cmdline failed: %s\n", cmdline);
        return false;
    }
    return true;
}

int popen_cmd(char *cmd, char *mode, char *buf, char bufSize)
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
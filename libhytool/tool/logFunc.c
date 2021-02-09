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

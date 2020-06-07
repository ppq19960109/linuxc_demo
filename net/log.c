#include <stdio.h>
#include <stdarg.h>
#include "log.h"

void log_printf(char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
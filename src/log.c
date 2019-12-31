#include "log.h"
#include "commom.h"

void log_printf(char* format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
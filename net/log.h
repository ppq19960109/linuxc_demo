#ifndef _LOG_H_
#define _LOG_H_

#define NORMAL_COLOR "\033[0m"
#define YELLOW "\033[1;33m"

#define log_color(color, fmt, ...)                                            \
    log_printf(color "%s-[%s-%d]: " fmt NORMAL_COLOR, __FUNCTION__, __FILE__, \
               __LINE__, ##__VA_ARGS__)
#define log_debug(fmt, ...) log_color(YELLOW, fmt, ##__VA_ARGS__)

void log_printf(char* format, ...);

#endif
#ifndef __LOGFUNC_H_
#define __LOGFUNC_H_

#define NONE "\e[0m\n"
#define BLACK "\e[0;30m"
#define L_BLACK "\e[1;30m"
#define RED "\e[0;31m"
#define L_RED "\e[1;31m"
#define GREEN "\e[0;32m"
#define L_GREEN "\e[1;32m"
#define BROWN "\e[0;33m"
#define YELLOW "\e[1;33m"
#define BLUE "\e[0;34m"
#define L_BLUE "\e[1;34m"
#define PURPLE "\e[0;35m"
#define L_PURPLE "\e[1;35m"
#define CYAN "\e[0;36m"
#define L_CYAN "\e[1;36m"
#define GRAY "\e[0;37m"
#define WHITE "\e[1;37m"
#define BOLD "\e[1m"
#define UNDERLINE "\e[4m"
#define BLINK "\e[5m"
#define REVERSE "\e[7m"
#define HIDE "\e[8m"
#define CLEAR "\e[2J"
#define CLRLINE "\r\e[K" //or "\e[1K\r"

#define log_color(color, fmt, ...)                      \
    logPrintf(color "[%s-%d]: " fmt NONE, __FUNCTION__, \
              __LINE__, ##__VA_ARGS__)

#define logDebug(fmt, ...) log_color(BLUE, fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...) log_color(GREEN, fmt, ##__VA_ARGS__)
#define logWarn(fmt, ...) log_color(YELLOW, fmt, ##__VA_ARGS__)
#define logError(fmt, ...) log_color(RED, fmt, ##__VA_ARGS__)

void logPrintf(char *format, ...);
const int consoleRun(const char *cmdline);
int popenRun(const char *cmd, const char *mode, char *buf, int bufSize);
#endif

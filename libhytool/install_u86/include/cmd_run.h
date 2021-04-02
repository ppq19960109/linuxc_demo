#ifndef __CMD_FUN_H_
#define __CMD_FUN_H_

int systemRun(const char *cmdline);
int popenRun(const char *cmdline, const char *mode, char *buf, int bufSize);
#endif

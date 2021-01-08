#ifndef _SIGNALQUIT_H_
#define _SIGNALQUIT_H_

typedef int (*Quit_cb)(void);
void signalQuit(void);
void registerQuitCb(Quit_cb quitCb);
#endif
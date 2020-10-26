#ifndef _LOCAL_CALLBACK_H_
#define _LOCAL_CALLBACK_H_

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void (*openCallback_t)(void);
    void register_openCallback(openCallback_t callback);
    void run_openCallback(void);

    typedef int (*writeCallback_t)(char *, unsigned int);
    void register_writeCallback(writeCallback_t callback);
    int run_writeCallback(char *, unsigned int);

    typedef void (*closeCallback_t)(void);
    void register_closeCallback(closeCallback_t callback);
    void run_closeCallback(void);
#ifdef __cplusplus
}
#endif
#endif
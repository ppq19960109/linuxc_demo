#ifndef _DATABASE_H_
#define _DATABASE_H_

#ifdef __cplusplus
extern "C"
{
#endif
    void databaseInit(void);
    int databaseClose(void);
    int databseReset(void);

    int insertDatabse(const char *devId, const char *modelId);
    int deleteDatabse(const char *devId);
    int selectDatabse(int (*addFunc)(const char *, const char *));
#ifdef __cplusplus
}
#endif
#endif
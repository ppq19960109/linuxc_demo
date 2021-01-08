#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sqlite3.h"

#include "frameCb.h"
#include "logFunc.h"
#include "hylinkListFunc.h"
#include "database.h"

#define DB_NAME "hylink.db"
typedef struct
{
    sqlite3 *db;

} sqlHandle_t;

sqlHandle_t sqlHandle;

static const char *hylinkCreateTable =
    "CREATE TABLE hylink(\
    ID INTEGER PRIMARY KEY AUTOINCREMENT,\
    DEVID TEXT UNIQUE NOT NULL,\
    MODELID TEXT NOT NULL);";

static const char *hylinkSqlSelect = "SELECT * FROM hylink;";
static const char *hylinkSqlInsert = "INSERT OR REPLACE INTO hylink VALUES (NULL,?,?);";
static const char *hylinkSqlDelete = "DELETE FROM hylink WHERE DEVID = ?;";

static const char *hylinkDeleteTable = "DELETE FROM hylink;";
static const char *hylinkDeleteTableSeq = "DELETE FROM sqlite_sequence WHERE name = 'hylink';";

int deleteDatabseTable(void)
{
    char *errMsg = NULL;
    int rc = sqlite3_exec(sqlHandle.db, hylinkDeleteTable, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc)
    {
        logError("sqlite3_exec errmsg:%s\n", errMsg);
        sqlite3_free(errMsg);
    }
    rc = sqlite3_exec(sqlHandle.db, hylinkDeleteTableSeq, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc)
    {
        logError("sqlite3_exec errmsg:%s\n", errMsg);
        sqlite3_free(errMsg);
    }
    return 0;
}

int deleteDatabse(void *dev, void *model)
{
    const char *devId = (const char *)dev;
    // const char *modelId = (const char *)model;

    sqlite3_stmt *pstmt;
    const char *pzTail;
    int rc = sqlite3_prepare_v2(sqlHandle.db, hylinkSqlDelete, -1, &pstmt, &pzTail);
    if (SQLITE_OK != rc)
    {
        logError("sqlite3_prepare_v2:%s\n", sqlite3_errmsg(sqlHandle.db));
        sqlite3_finalize(pstmt);
    }

    sqlite3_bind_text(pstmt, 1, devId, strlen(devId), NULL);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);

    sqlite3_finalize(pstmt);
    hylinkListDel(devId);
    return 0;
}

//--------------------------------
int addDevList(const char *devId, const char *modelId)
{
    if (devId == NULL || modelId == NULL)
        return -1;
    HylinkDev *hylinkDev = (HylinkDev *)malloc(sizeof(HylinkDev));
    memset(hylinkDev, 0, sizeof(HylinkDev));
    strcpy(hylinkDev->DeviceId, devId);
    strcpy(hylinkDev->ModelId, modelId);
    hylinkListAdd(hylinkDev);
    return 0;
}

int insertDatabse(void *dev, void *model)
{
    const char *devId = (const char *)dev;
    const char *modelId = (const char *)model;

    sqlite3_stmt *pstmt;
    const char *pzTail;
    int rc = sqlite3_prepare_v2(sqlHandle.db, hylinkSqlInsert, -1, &pstmt, &pzTail);
    if (SQLITE_OK != rc)
    {
        logError("sqlite3_prepare_v2:%s\n", sqlite3_errmsg(sqlHandle.db));
        sqlite3_finalize(pstmt);
    }

    sqlite3_bind_text(pstmt, 1, devId, strlen(devId), NULL);
    sqlite3_bind_text(pstmt, 2, modelId, strlen(modelId), NULL);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);

    sqlite3_finalize(pstmt);
    addDevList(devId, modelId);
    return 0;
}

int selectDatabse(void *func)
{
    int (*addFunc)(const char *, const char *) = (int (*)(const char *, const char *))func;

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(sqlHandle.db, hylinkSqlSelect, -1, &stmt, NULL);
    if (SQLITE_OK != rc)
    {
        logError("sqlite3_prepare_v2\n");
        sqlite3_finalize(stmt);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        //一列一列地去读取每一条记录 1表示列
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *devid = sqlite3_column_text(stmt, 1);
        const unsigned char *modelid = sqlite3_column_text(stmt, 2);
        logDebug("ID:%d devid:%s modelid:%s", id, devid, modelid);
        if (addFunc != NULL)
            addFunc((const char *)devid, (const char *)modelid);
    }
    return sqlite3_finalize(stmt);
}
//---------------------------------
int databaseClose(void)
{
    return sqlite3_close_v2(sqlHandle.db);
}

void databaseInit(void)
{
    registerSystemCb(databaseClose, DATABASE_CLOSE);
    registerSystemCb(deleteDatabseTable, DATABASE_RESET);
    registerCmdCb(deleteDatabse, DATABASE_DELETE);
    registerCmdCb(insertDatabse, DATABASE_INSERT);

    char *errMsg = NULL;
    int rc = sqlite3_open_v2(DB_NAME, &sqlHandle.db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    if (SQLITE_OK != rc)
    {
        logError("sqlite3_open_v2 errmsg:%s", sqlite3_errmsg(sqlHandle.db));
        sqlite3_close_v2(sqlHandle.db);
    }

    rc = sqlite3_exec(sqlHandle.db, hylinkCreateTable, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc)
    {
        logError("sqlite3_exec errmsg:%s\n", errMsg);
        sqlite3_free(errMsg);
    }
    selectDatabse(addDevList);
}

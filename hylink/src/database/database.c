#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sqlite3.h"

#include "database.h"


#define DB_NAME "hylink.db"
#define TABLE_NAME "hylink"
typedef struct
{
    sqlite3 *db;

} sqlHandle_t;

sqlHandle_t sqlHandle;

static const char *createTable =
    "CREATE TABLE " TABLE_NAME "(\
    ID INTEGER PRIMARY KEY AUTOINCREMENT,\
    DEVID TEXT UNIQUE NOT NULL,\
    MODELID TEXT NOT NULL);";

static const char *sqlSelect = "SELECT * FROM " TABLE_NAME ";";
static const char *sqlInsert = "INSERT OR REPLACE INTO " TABLE_NAME " VALUES (NULL,?,?);";
static const char *sqlDelete = "DELETE FROM " TABLE_NAME " WHERE DEVID = ?;";

static const char *sqlDeleteTable = "DELETE FROM " TABLE_NAME ";";
static const char *sqlDeleteTableSeq = "DELETE FROM sqlite_sequence WHERE name = " TABLE_NAME ";";

int databseReset(void)
{
    char *errMsg = NULL;
    int rc = sqlite3_exec(sqlHandle.db, sqlDeleteTable, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_exec errmsg:%s\n", errMsg);
        sqlite3_free(errMsg);
    }
    rc = sqlite3_exec(sqlHandle.db, sqlDeleteTableSeq, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_exec errmsg:%s\n", errMsg);
        sqlite3_free(errMsg);
    }
    return 0;
}

int deleteDatabse(const char *devId)
{

    sqlite3_stmt *pstmt;
    const char *pzTail;
    int rc = sqlite3_prepare_v2(sqlHandle.db, sqlDelete, -1, &pstmt, &pzTail);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_prepare_v2:%s\n", sqlite3_errmsg(sqlHandle.db));
        sqlite3_finalize(pstmt);
    }

    sqlite3_bind_text(pstmt, 1, devId, strlen(devId), NULL);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);

    sqlite3_finalize(pstmt);
    return 0;
}

int insertDatabse(const char *devId, const char *modelId)
{

    sqlite3_stmt *pstmt;
    const char *pzTail;
    int rc = sqlite3_prepare_v2(sqlHandle.db, sqlInsert, -1, &pstmt, &pzTail);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_prepare_v2:%s\n", sqlite3_errmsg(sqlHandle.db));
        sqlite3_finalize(pstmt);
    }

    sqlite3_bind_text(pstmt, 1, devId, strlen(devId), NULL);
    sqlite3_bind_text(pstmt, 2, modelId, strlen(modelId), NULL);

    sqlite3_step(pstmt);
    sqlite3_reset(pstmt);

    sqlite3_finalize(pstmt);
    return 0;
}

int selectDatabse(int (*addFunc)(const char *, const char *))
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(sqlHandle.db, sqlSelect, -1, &stmt, NULL);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_prepare_v2\n");
        sqlite3_finalize(stmt);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        //一列一列地去读取每一条记录 1表示列
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *devid = sqlite3_column_text(stmt, 1);
        const unsigned char *modelid = sqlite3_column_text(stmt, 2);
        printf("ID:%d devid:%s modelid:%s\n", id, devid, modelid);
        if (addFunc != NULL)
            addFunc((const char *)devid, (const char *)modelid);
    }
    return sqlite3_finalize(stmt);
}

static int databse_table_init(void)
{

    char *errMsg = NULL;
    int rc = sqlite3_exec(sqlHandle.db, createTable, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_exec errmsg:%s\n", errMsg);
        sqlite3_free(errMsg);
    }
    return rc;
}
//---------------------------------
int databaseClose(void)
{
    return sqlite3_close_v2(sqlHandle.db);
}

void databaseInit(void)
{

    int rc = sqlite3_open_v2(DB_NAME, &sqlHandle.db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    if (SQLITE_OK != rc)
    {
        printf("sqlite3_open_v2 errmsg:%s\n", sqlite3_errmsg(sqlHandle.db));
        sqlite3_close_v2(sqlHandle.db);
    }
    databse_table_init();
}

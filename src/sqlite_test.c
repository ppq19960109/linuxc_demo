#include "sqlite_test.h"
#include "commom.h"
#include "sqlite3.h"

struct sqlite_db_t {
    sqlite3 *db;
};

struct sqlite_db_t sqlite_db;

static const char *sql_createtable =
    "CREATE TABLE test(\
    ID INTEGER PRIMARY KEY,\
    NAME VARCHAR(16),\
    AGE INTEGER,\
    GENDER VARCHAR(4),\
    OTHERS TEXT);";

char *sql_insert = "INSERT INTO test VALUES (4,'ppq',22,'man','hello')";

char *sql_select = "SELECT * FROM test";

static int callback(void *data, int argc, char **argv, char **azColName) {
    int i;
    log_debug("data:%s\n", data);

    for (i = 0; i < argc; i++) {
        printf("%s:%s ", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int sqlite_table() {
    char *errmsg = NULL;

    int rc = sqlite3_exec(sqlite_db.db, sql_insert, NULL, NULL, &errmsg);
    if (SQLITE_OK != rc) {
        log_debug("sqlite3_exec errmsg:%s\n", errmsg);
        sqlite3_free(errmsg);
    }

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(sqlite_db.db, sql_select, -1, &stmt, NULL);
    if (SQLITE_OK != rc) {
        log_debug("sqlite3_prepare_v2\n");
        sqlite3_finalize(stmt);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        //一列一列地去读取每一条记录 1表示列
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char  *name = sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const unsigned char  *gender = sqlite3_column_text(stmt, 3);
        const unsigned char  *others = sqlite3_column_text(stmt, 4);
        log_debug("ID:%d name:%s age:%d gender:%s others:%s\n",id,name,age,gender,others);
    }
    sqlite3_finalize(stmt);

    // rc = sqlite3_exec(sqlite_db.db, sql_select, callback, "select test",
    // &errmsg); if (SQLITE_OK != rc) {
    //     log_debug("sqlite3_exec errmsg:%s\n", errmsg);
    //     sqlite3_free(errmsg);
    // }



    // char **pResult;
    // int nRow;
    // int nCol;

    // rc = sqlite3_get_table(sqlite_db.db, sql_select, &pResult, &nRow, &nCol,
    //                        &errmsg);
    // if (rc != SQLITE_OK) {
    //     log_debug("sqlite3_exec errmsg:%s\n", errmsg);
    //     sqlite3_free(errmsg);
    // }

    // int nIndex = nCol;
    // for (int i = 0; i < nRow; i++) {
    //     log_debug("Row:%d ", nRow);
    //     for (int j = 0; j < nCol; j++) {
    //         printf("%s:%s ", pResult[j], pResult[nIndex]);
    //         ++nIndex;
    //     }
    //     printf("\n");
    // }

    return 0;
}

int sqlite_test_init() {
    char *zErrMsg = NULL;
    int rc = sqlite3_open_v2("test.db", &sqlite_db.db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    if (rc) {
        log_debug("sqlite3_open_v2 errmsg:%s\n", sqlite3_errmsg(sqlite_db.db));
        sqlite3_close_v2(sqlite_db.db);
    }

    rc = sqlite3_exec(sqlite_db.db, sql_createtable, NULL, NULL, &zErrMsg);
    if (SQLITE_OK != rc) {
        log_debug("sqlite3_exec errmsg:%s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite_table();

    sqlite3_close_v2(sqlite_db.db);
    return 0;
}

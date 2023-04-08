#include <stdio.h>

#include "sqlite3.h"

/* print a record from table outputed by sql statement */
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int test_sqlite3() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
        return (1);
    }
    rc = sqlite3_open(argv[1], &db); /* open database */
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }
    rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg); /* execute SQL statement */
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db); /* close database */
    return 0;
}

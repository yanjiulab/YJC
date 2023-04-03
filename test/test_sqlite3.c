#include "db.h"
#include "test.h"

void test_sqlite3() {
    db_init("my.db");
    struct sqlite3_stmt* ss = db_prepare(
        "CREATE TABLE IF NOT EXISTS member (name TEXT NOT NULL, datestamp DATETIME DEFAULT CURRENT_TIMESTAMP);");
    db_run(ss);
    db_close();
}
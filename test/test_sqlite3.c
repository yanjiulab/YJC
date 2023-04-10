
// The following two objects and eight methods comprise the essential elements of the SQLite interface:
// sqlite3 → The database connection object. Created by sqlite3_open() and destroyed by sqlite3_close().
// sqlite3_stmt → The prepared statement object. Created by sqlite3_prepare() and destroyed by sqlite3_finalize().
// sqlite3_open() → Open a connection to a new or existing SQLite database. The constructor for sqlite3.
// sqlite3_prepare() → Compile SQL text into byte-code that will do the work of querying or updating the database. The
// constructor for sqlite3_stmt.
// sqlite3_bind() → Store application data into parameters of the original SQL.
// sqlite3_step() → Advance an sqlite3_stmt to the next result row or to completion.
// sqlite3_column() → Column values in the current result row for an sqlite3_stmt.
// sqlite3_finalize() → Destructor for sqlite3_stmt.
// sqlite3_close() → Destructor for sqlite3.
// sqlite3_exec() → A wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(), and
// sqlite3_finalize() for a string of one or more SQL statements.

#include "db.h"
#include "log.h"
#include "test.h"
void test_sqlite3() {
    int rc;

    // Open the database
    db_init("my.db");

    // Delete table
    db_execute("DROP TABLE IF EXISTS member;");

    // Create table if not exist
    db_execute(
        "CREATE TABLE IF NOT EXISTS member("
        " name TEXT PRIMARY KEY NOT NULL,"
        " age INT NOT NULL,"
        " weight REAL NOT NULL,"
        " datestamp DATETIME DEFAULT CURRENT_TIMESTAMP);");

    // Clear table content
    // db_execute("DELETE FROM member;");

    // Insert some rows
    struct sqlite3_stmt* ss;
    char* names[5] = {"Alice", "Bob", "Corol", "David", "Ella"};
    int age = 18;
    double weight = 73.52;

    ss = db_prepare(
        "INSERT INTO member"
        "  (name, age, weight)"
        "VALUES"
        "  (?, ?, ?);");
    for (int i = 0; i < 5; i++) {
        db_bindf(ss, "%s%d%f", names[i], strlen(names[i]), age, weight);
        db_run(ss);
        db_reset(ss);
        age++;
        weight += 0.16;
    }
    db_finalize(ss);

    ss = db_prepare(" select name,age,weight,datestamp from member;");
    char** name, **ts;
    int a;
    double w;
    while (db_run(ss)) {
        db_loadf(ss, "%s %d %f %s", name, &a, &w, ts);
        log_info("%s\t%d\t%f", *name, a, w);
        // *t = sqlite3_column_text(ss, 3);
        // log_info("%s", *t);
    }
    db_finalize(ss);

    // select (julianday('2023-04-10 17:16:00') - julianday('1970-01-01')) * 86400;
    // ss = db_prepare("SELECT datestamp FROM member;");
    // while (db_run(ss)) {
    //     db_loadf(ss, "%s", name);
    //     log_info("%s", *name);
    //     // *t = sqlite3_column_text(ss, 3);
    //     // log_info("%s", *t);
    // }
    // db_finalize(ss);




    // int i = 0, j = 0;
    // int nrow, ncol, idx;
    // char **result, *errmsg;
    // rc = sqlite3_get_table(db(), "SELECT * FROM member;", &result, &nrow, &ncol, &errmsg);
    // if (rc) {
    //     printf("error: %s\n", errmsg);
    // }
    // for (i = 0; i <= nrow; i++) {
    //     for (j = 0; j < ncol; j++) {
    //         printf("%s\t", result[i * ncol + j] ? result[i * ncol + j] : "(null)");
    //     }
    //     printf("\n");
    // }
    // sqlite3_free_table(result);

    // Close the database
    db_close();
}

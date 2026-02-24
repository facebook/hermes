/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef SQTEST_SQMOCK_H
#define SQTEST_SQMOCK_H

// sqlite3.h

#define SQLITE_OK 0
#define SQLITE_ROW 100
#define SQLITE_DONE 101

#define SQLITE_INTEGER 1
#define SQLITE_FLOAT 2
#define SQLITE_TEXT 3

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

#ifdef __cplusplus
extern "C" {
#endif
int sqlite3_open(const char *filename, sqlite3 **db);
int sqlite3_prepare_v2(
    sqlite3 *db,
    const char *sql,
    int nByte,
    sqlite3_stmt **stmt,
    const char **pzTail);
int sqlite3_step(sqlite3_stmt *stmt);
int sqlite3_column_count(sqlite3_stmt *stmt);
const char *sqlite3_column_name(sqlite3_stmt *stmt, int N);
int sqlite3_column_type(sqlite3_stmt *stmt, int iCol);
int sqlite3_column_int(sqlite3_stmt *stmt, int iCol);
double sqlite3_column_double(sqlite3_stmt *stmt, int iCol);
const unsigned char *sqlite3_column_text(sqlite3_stmt *stmt, int iCol);
int sqlite3_column_bytes(sqlite3_stmt *, int iCol);
int sqlite3_finalize(sqlite3_stmt *stmt);
int sqlite3_close(sqlite3 *db);
#ifdef __cplusplus
}
#endif

#endif // SQTEST_SQMOCK_H

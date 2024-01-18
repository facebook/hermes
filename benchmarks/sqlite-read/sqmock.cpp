/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "sqmock.h"

#include <cstdlib>
#include <cstring>
#include <random>

#define MIN_TEXT 7
#define MAX_TEXT 23
#define NUM_ROWS 300000
#define GEN_EACH_ROW false

struct sqlite3 {
  std::mt19937 generator;
  std::uniform_int_distribution<int> int_distribution{1, 100};
  std::uniform_real_distribution<double> real_distribution{1.0, 100.0};
};

typedef struct {
  int id;
  int text_length; // We use the same length for all text columns.
  char v1[MAX_TEXT + 1];
  char v2[MAX_TEXT + 1];
  char v3[MAX_TEXT + 1];
  char v4[MAX_TEXT + 1];
  char v5[MAX_TEXT + 1];
  int v6;
  int v7;
  int v8;
  int v9;
  int v10;
  double v11;
  double v12;
  double v13;
  double v14;
} Row;

struct sqlite3_stmt {
  sqlite3 *db;
  int current_row;
  Row row;
};

int sqlite3_open(const char *filename, sqlite3 **db) {
  *db = new sqlite3();
  return SQLITE_OK;
}

int sqlite3_prepare_v2(
    sqlite3 *db,
    const char *sql,
    int nByte,
    sqlite3_stmt **stmt,
    const char **pzTail) {
  *stmt = new sqlite3_stmt();
  (*stmt)->db = db;
  (*stmt)->current_row = 0;
  return SQLITE_OK;
}

int sqlite3_step(sqlite3_stmt *stmt) {
  sqlite3 *db = stmt->db;
  if (stmt->current_row >= NUM_ROWS) {
    return SQLITE_DONE;
  }
  // Only generate the first row to save time.
  if (!GEN_EACH_ROW && stmt->current_row) {
    ++stmt->current_row;
    return SQLITE_ROW;
  }
  stmt->row.id = stmt->current_row;
  int text_length = MIN_TEXT +
      db->int_distribution(db->generator) % (MAX_TEXT - MIN_TEXT + 1);
  stmt->row.text_length = text_length;
  char c = 'a' + db->int_distribution(db->generator) % 26;
  // Set all text columns to the same value, repeating the character.
  memset(stmt->row.v1, c, text_length);
  memset(stmt->row.v2, c, text_length);
  memset(stmt->row.v3, c, text_length);
  memset(stmt->row.v4, c, text_length);
  memset(stmt->row.v5, c, text_length);

  stmt->row.v1[text_length] = '\0';
  stmt->row.v2[text_length] = '\0';
  stmt->row.v3[text_length] = '\0';
  stmt->row.v4[text_length] = '\0';
  stmt->row.v5[text_length] = '\0';
  stmt->row.v6 = db->int_distribution(db->generator);
  stmt->row.v7 = db->int_distribution(db->generator);
  stmt->row.v8 = db->int_distribution(db->generator);
  stmt->row.v9 = db->int_distribution(db->generator);
  stmt->row.v10 = db->int_distribution(db->generator);
  stmt->row.v11 = db->real_distribution(db->generator);
  stmt->row.v12 = db->real_distribution(db->generator);
  stmt->row.v13 = db->real_distribution(db->generator);
  stmt->row.v14 = db->real_distribution(db->generator);
  stmt->current_row++;
  return SQLITE_ROW;
}

int sqlite3_column_count(sqlite3_stmt *stmt) {
  return 14;
}

const char *sqlite3_column_name(sqlite3_stmt *stmt, int N) {
  switch (N) {
    case 0:
      return "id";
    case 1:
      return "v1";
    case 2:
      return "v2";
    case 3:
      return "v3";
    case 4:
      return "v4";
    case 5:
      return "v5";
    case 6:
      return "v6";
    case 7:
      return "v7";
    case 8:
      return "v8";
    case 9:
      return "v9";
    case 10:
      return "v10";
    case 11:
      return "v11";
    case 12:
      return "v12";
    case 13:
      return "v13";
    case 14:
      return "v14";
    default:
      return NULL;
  }
}

int sqlite3_column_type(sqlite3_stmt *stmt, int iCol) {
  if (iCol == 0 || (iCol >= 6 && iCol <= 10)) {
    return SQLITE_INTEGER;
  } else if (iCol >= 11 && iCol <= 14) {
    return SQLITE_FLOAT;
  } else if (iCol >= 1 && iCol <= 5) {
    return SQLITE_TEXT;
  } else {
    return 0;
  }
}

int sqlite3_column_int(sqlite3_stmt *stmt, int iCol) {
  switch (iCol) {
    case 0:
      return stmt->row.id;
    case 6:
      return stmt->row.v6;
    case 7:
      return stmt->row.v7;
    case 8:
      return stmt->row.v8;
    case 9:
      return stmt->row.v9;
    case 10:
      return stmt->row.v10;
    default:
      return 0;
  }
}

double sqlite3_column_double(sqlite3_stmt *stmt, int iCol) {
  switch (iCol) {
    case 11:
      return stmt->row.v11;
    case 12:
      return stmt->row.v12;
    case 13:
      return stmt->row.v13;
    case 14:
      return stmt->row.v14;
    default:
      return 0.0;
  }
}

const unsigned char *sqlite3_column_text(sqlite3_stmt *stmt, int iCol) {
  switch (iCol) {
    case 1:
      return (const unsigned char *)stmt->row.v1;
    case 2:
      return (const unsigned char *)stmt->row.v2;
    case 3:
      return (const unsigned char *)stmt->row.v3;
    case 4:
      return (const unsigned char *)stmt->row.v4;
    case 5:
      return (const unsigned char *)stmt->row.v5;
    default:
      return NULL;
  }
}

int sqlite3_column_bytes(sqlite3_stmt *stmt, int iCol) {
  switch (iCol) {
    case 1:
      return stmt->row.text_length;
    case 2:
      return stmt->row.text_length;
    case 3:
      return stmt->row.text_length;
    case 4:
      return stmt->row.text_length;
    case 5:
      return stmt->row.text_length;
    default:
      return 0;
  }
}

int sqlite3_finalize(sqlite3_stmt *stmt) {
  free(stmt);
  return SQLITE_OK;
}

int sqlite3_close(sqlite3 *db) {
  free(db);
  return SQLITE_OK;
}

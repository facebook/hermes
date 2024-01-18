/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "sqmock.h"

typedef std::variant<int, double, std::string> SqliteData;

#if 0
using MapTy = std::map<std::string, SqliteData>;
#else
using MapTy = std::unordered_map<std::string, SqliteData>;
#endif

int main() {
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int res;
  std::vector<MapTy> results;

  try {
    // Open the database
    res = sqlite3_open("largeDB.sqlite", &db);
    if (res != SQLITE_OK)
      throw res;

    std::clock_t start = std::clock();

    // Prepare the SQL statement
    res = sqlite3_prepare_v2(db, "SELECT * FROM Test", -1, &stmt, NULL);
    if (res != SQLITE_OK)
      throw res;

    // Loop through all rows in the table
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      MapTy res;
      int count = sqlite3_column_count(stmt);
      for (int i = 0; i < count; i++) {
        const char *namePtr = sqlite3_column_name(stmt, i);
        std::string name(namePtr);
        int type = sqlite3_column_type(stmt, i);
        if (type == SQLITE_INTEGER) {
          res[name] = sqlite3_column_int(stmt, i);
        } else if (type == SQLITE_FLOAT) {
          res[name] = sqlite3_column_double(stmt, i);
        } else if (type == SQLITE_TEXT) {
          const unsigned char *textPtr = sqlite3_column_text(stmt, i);
          std::string text(reinterpret_cast<const char *>(textPtr));
          res[name] = text;
        }
      }
      results.push_back(std::move(res));
    }
    std::clock_t end = std::clock();

    std::cout << results.size() << std::endl;
    std::cout << "Time: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << " ms"
              << std::endl;

    // Print the contents of res by iterating over the map in each row.
    /*
            for (auto &row : results) {
                for (auto &pair : row) {
                    std::cout << pair.first << ": ";
                    if (std::holds_alternative<int>(pair.second)) {
                        std::cout << std::get<int>(pair.second);
                    } else if (std::holds_alternative<double>(pair.second)) {
                        std::cout << std::get<double>(pair.second);
                    } else if (std::holds_alternative<std::string>(pair.second))
       { std::cout << std::get<std::string>(pair.second);
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }
    */
  } catch (int e) {
    std::cerr << "Error: " << e << std::endl;
  }
  // Finalize the statement and close the database
  if (stmt)
    sqlite3_finalize(stmt);
  if (db)
    sqlite3_close(db);

  return 0;
}

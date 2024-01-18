# Reading an SQLite3 DB into memory

This benchmark compares the time it takes to read an sqlite3 database into memory in C++ and Static Hermes.

The actual DB API is mocked using a very simple implementation in `sqmock.cpp`, implementing only a few 
SQLite3 APIs.

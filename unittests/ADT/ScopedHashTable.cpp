/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "hermes/ADT/ScopedHashTable.h"

namespace {

using Table = hermes::ScopedHashTable<llvm::StringRef, llvm::StringRef>;
using Scope = hermes::ScopedHashTableScope<llvm::StringRef, llvm::StringRef>;

TEST(ScopedHashTable, SmokeTest) {
  Table table;
  Scope scope(table);
  table.insert("foo", "bar");
  EXPECT_EQ("bar", table.lookup("foo"));
}

TEST(ScopedHashTable, Nesting) {
  Table table;
  Scope outer(table);
  table.insert("key", "outer");
  EXPECT_EQ("outer", table.lookup("key"));
  {
    Scope inner(table);
    table.insert("key", "inner");
    EXPECT_EQ("inner", table.lookup("key"));
  }
  EXPECT_EQ("outer", table.lookup("key"));
}

TEST(ScopedHashTable, Overwrite) {
  Table table;
  Scope outer(table);
  table.insert("key", "foo");
  EXPECT_EQ("foo", table.lookup("key"));
  table.insert("key", "outer");
  EXPECT_EQ("outer", table.lookup("key"));
  {
    Scope inner(table);
    table.insert("key", "foo");
    EXPECT_EQ("foo", table.lookup("key"));
    table.insert("key", "inner");
    EXPECT_EQ("inner", table.lookup("key"));
  }
  EXPECT_EQ("outer", table.lookup("key"));
}

TEST(ScopedHashTable, Flatten) {
  Table table;
  Scope outer(table);
  table.insert("out", "outer");
  {
    Scope inner(table);
    table.insert("in", "inner");
    auto map = table.flatten();
    EXPECT_EQ(2u, map->size());
    EXPECT_EQ("outer", map->lookup("out"));
    EXPECT_EQ("inner", map->lookup("in"));
  }
}

TEST(ScopedHashTable, GetKeysByScope) {
  Table table;
  Scope outer(table);
  table.insert("out", "outer");
  table.insert("in", "trash");
  {
    Scope inner(table);
    table.insert("in", "inner");
    auto scopes = table.getKeysByScope();
    EXPECT_EQ(2u, scopes->size());
    EXPECT_EQ(1u, scopes->at(0).size());
    EXPECT_EQ(1u, scopes->at(1).size());
    EXPECT_EQ("in", scopes->at(0)[0]);
    EXPECT_EQ("out", scopes->at(1)[0]);
  }
}

} // anonymous namespace

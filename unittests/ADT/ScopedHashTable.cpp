/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "hermes/ADT/ScopedHashTable.h"

namespace {

using Table = hermes::ScopedHashTable<llvh::StringRef, llvh::StringRef>;
using Scope = hermes::ScopedHashTableScope<llvh::StringRef, llvh::StringRef>;

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

TEST(ScopedHashTable, SetInCurrentScope) {
  Table table;
  Scope outer(table);
  table.insert("foo", "true");
  {
    Scope inner(table);
    EXPECT_EQ("true", table.lookup("foo"));
    table.setInCurrentScope("foo", "false");
    EXPECT_EQ("false", table.lookup("foo"));
    table.setInCurrentScope("foo", "true");
    EXPECT_EQ("true", table.lookup("foo"));
    table.setInCurrentScope("foo", "false");
    EXPECT_EQ("false", table.lookup("foo"));
  }
  EXPECT_EQ("true", table.lookup("foo"));
}

TEST(ScopedHashTable, EraseFromCurrentScope) {
  Table table{};

  // Try to erase missing element with no scopes.
  EXPECT_FALSE(table.eraseFromCurrentScope("foo"));

  Scope outer(table);
  // Try to erase missing element with a scope.
  EXPECT_FALSE(table.eraseFromCurrentScope("foo"));

  table.insert("foo", "10");
  {
    Scope inner(table);
    // Try to erase element from a parent scope.
    EXPECT_FALSE(table.eraseFromCurrentScope("foo"));
  }

  // Erase the only element in the only scope.
  EXPECT_TRUE(table.eraseFromCurrentScope("foo"));
  EXPECT_FALSE(table.eraseFromCurrentScope("foo"));

  table.insert("foo", "10");
  table.insert("bar", "20");

  // Erase the last added element.
  EXPECT_TRUE(table.eraseFromCurrentScope("bar"));
  EXPECT_FALSE(table.eraseFromCurrentScope("bar"));

  table.insert("bar", "20");

  // Erase the first added element.
  EXPECT_TRUE(table.eraseFromCurrentScope("foo"));
  EXPECT_FALSE(table.eraseFromCurrentScope("foo"));

  // Erase the remaining element.
  EXPECT_TRUE(table.eraseFromCurrentScope("bar"));
  EXPECT_FALSE(table.eraseFromCurrentScope("bar"));

  table.insert("1", "10");
  table.insert("2", "20");
  table.insert("3", "30");

  // Erase the middle element
  EXPECT_TRUE(table.eraseFromCurrentScope("2"));
  EXPECT_FALSE(table.eraseFromCurrentScope("2"));
  // Erase the rest.
  EXPECT_TRUE(table.eraseFromCurrentScope("1"));
  EXPECT_FALSE(table.eraseFromCurrentScope("1"));
  EXPECT_TRUE(table.eraseFromCurrentScope("3"));
  EXPECT_FALSE(table.eraseFromCurrentScope("3"));

  table.insert("1", "10");
  table.insert("2", "20");
  table.insert("3", "30");
  {
    Scope inner(table);

    table.insert("1", "10");
    table.insert("2", "20");
    table.insert("3", "30");

    // Erase the middle element in a nested scope, with a shadowed element.
    EXPECT_TRUE(table.eraseFromCurrentScope("2"));
    EXPECT_FALSE(table.eraseFromCurrentScope("2"));

    {
      auto scopes = table.getKeysByScope();
      EXPECT_EQ(2u, scopes->size());
      auto &top = scopes->at(0);
      EXPECT_EQ(2u, top.size());
      EXPECT_TRUE(std::find(top.begin(), top.end(), "1") != top.end());
      EXPECT_TRUE(std::find(top.begin(), top.end(), "3") != top.end());
    }
  }
  {
    auto scopes = table.getKeysByScope();
    EXPECT_EQ(1u, scopes->size());
    auto &top = scopes->at(0);
    EXPECT_EQ(3u, top.size());
    EXPECT_TRUE(std::find(top.begin(), top.end(), "1") != top.end());
    EXPECT_TRUE(std::find(top.begin(), top.end(), "2") != top.end());
    EXPECT_TRUE(std::find(top.begin(), top.end(), "3") != top.end());
  }
}

} // anonymous namespace

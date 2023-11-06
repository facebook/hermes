/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "hermes/ADT/PersistentScopedMap.h"

namespace {

using Table = hermes::PersistentScopedMap<llvh::StringRef, llvh::StringRef>;
using Scope =
    hermes::PersistentScopedMapScope<llvh::StringRef, llvh::StringRef>;
using Ptr =
    hermes::PersistentScopedMapScopePtr<llvh::StringRef, llvh::StringRef>;

TEST(PersistentScopedMap, SmokeTest) {
  Table table;
  Scope scope(table);
  table.try_emplace("foo", "bar");
  EXPECT_EQ("bar", table.lookup("foo"));
}

TEST(PersistentScopedMap, Nesting) {
  Table table;
  Scope outer(table);
  table.try_emplace("key", "outer");
  EXPECT_EQ("outer", table.lookup("key"));
  {
    Scope inner(table);
    table.try_emplace("key", "inner");
    EXPECT_EQ("inner", table.lookup("key"));
  }
  EXPECT_EQ("outer", table.lookup("key"));
}

TEST(PersistentScopedMap, Overwrite) {
  Table table;
  Scope outer(table);
  table.put("key", "foo");
  EXPECT_EQ("foo", table.lookup("key"));
  table.put("key", "outer");
  EXPECT_EQ("outer", table.lookup("key"));
  {
    Scope inner(table);
    table.put("key", "foo");
    EXPECT_EQ("foo", table.lookup("key"));
    table.put("key", "inner");
    EXPECT_EQ("inner", table.lookup("key"));
  }
  EXPECT_EQ("outer", table.lookup("key"));
}

TEST(PersistentScopedMap, Flatten) {
  Table table;
  Scope outer(table);
  table.try_emplace("out", "outer");
  {
    Scope inner(table);
    table.try_emplace("in", "inner");
    auto map = table.test_flatten();
    EXPECT_EQ(2u, map->size());
    EXPECT_EQ("outer", map->lookup("out"));
    EXPECT_EQ("inner", map->lookup("in"));
  }
}

TEST(PersistentScopedMap, GetKeysByScope) {
  Table table;
  Scope outer(table);
  table.try_emplace("out", "outer");
  table.try_emplace("in", "trash");
  {
    Scope inner(table);
    table.try_emplace("in", "inner");
    auto scopes = table.test_getKeysByScope();
    EXPECT_EQ(2u, scopes->size());
    EXPECT_EQ(1u, scopes->at(0).size());
    EXPECT_EQ(1u, scopes->at(1).size());
    EXPECT_EQ("in", scopes->at(0)[0]);
    EXPECT_EQ("out", scopes->at(1)[0]);
  }
}

TEST(PersistentScopedMap, Put) {
  Table table;
  Scope outer(table);
  table.try_emplace("foo", "true");
  {
    Scope inner(table);
    EXPECT_EQ("true", table.lookup("foo"));
    table.try_emplace("foo", "false");
    EXPECT_EQ("false", table.lookup("foo"));
    table.try_emplace("foo", "true");
    EXPECT_EQ("false", table.lookup("foo"));
    table.put("foo", "false");
    EXPECT_EQ("false", table.lookup("foo"));
  }
  EXPECT_EQ("true", table.lookup("foo"));
}

TEST(PersistentScopedMap, FindInCurrentScope) {
  Table table;
  Scope outer(table);
  table.try_emplace("foo", "true");
  {
    Scope inner(table);
    table.try_emplace("bar", "true");
    EXPECT_EQ(nullptr, table.findInCurrentScope("foo"));
    EXPECT_EQ("true", *table.findInCurrentScope("bar"));
  }
  EXPECT_EQ("true", *table.findInCurrentScope("foo"));
}

TEST(PersistentScopedMap, Activate) {
  Ptr ptr;
  Table table;
  {
    Scope A(table);
    table.try_emplace("A", "a");
    table.try_emplace("key", "keyA");
    Scope B(table);
    table.try_emplace("B", "b");
    table.try_emplace("key", "keyB");
    {
      Scope C(table);
      table.try_emplace("C", "c");
      table.try_emplace("key", "keyC");
      Scope D(table);
      table.try_emplace("D", "d");
      table.try_emplace("key", "keyD");
      ptr = D.ptr();

      {
        auto map = table.test_flatten();
        EXPECT_EQ(5u, map->size());
        EXPECT_EQ("a", map->lookup("A"));
        EXPECT_EQ("b", map->lookup("B"));
        EXPECT_EQ("c", map->lookup("C"));
        EXPECT_EQ("d", map->lookup("D"));
        EXPECT_EQ("keyD", map->lookup("key"));
      }
    }
    {
      auto map = table.test_flatten();
      EXPECT_EQ(3u, map->size());
      EXPECT_EQ("a", map->lookup("A"));
      EXPECT_EQ("b", map->lookup("B"));
      EXPECT_EQ("keyB", map->lookup("key"));
    }
    Scope E(table);
    table.try_emplace("E", "e");
    table.try_emplace("key", "keyE");
    Scope F(table);
    table.try_emplace("F", "f");
    table.try_emplace("key", "keyF");
    Scope G(table);
    table.try_emplace("G", "g");
    table.try_emplace("key", "keyG");
    /*
                            -> C->D
                          /
      A(active)->B(active)
                          \
                            -> E(active)->F(active)->G(active)
     */
    {
      auto map = table.test_flatten();
      EXPECT_EQ(6u, map->size());
      EXPECT_EQ("a", map->lookup("A"));
      EXPECT_EQ("b", map->lookup("B"));
      EXPECT_EQ("e", map->lookup("E"));
      EXPECT_EQ("f", map->lookup("F"));
      EXPECT_EQ("g", map->lookup("G"));
      EXPECT_EQ("keyG", map->lookup("key"));
    }

    table.activateScope(E.ptr());
    {
      auto map = table.test_flatten();
      EXPECT_EQ(4u, map->size());
      EXPECT_EQ("a", map->lookup("A"));
      EXPECT_EQ("b", map->lookup("B"));
      EXPECT_EQ("e", map->lookup("E"));
      EXPECT_EQ("keyE", map->lookup("key"));
    }

    table.activateScope(F.ptr());
    {
      auto map = table.test_flatten();
      EXPECT_EQ(5u, map->size());
      EXPECT_EQ("a", map->lookup("A"));
      EXPECT_EQ("b", map->lookup("B"));
      EXPECT_EQ("e", map->lookup("E"));
      EXPECT_EQ("f", map->lookup("F"));
      EXPECT_EQ("keyF", map->lookup("key"));
    }
    table.activateScope(G.ptr());
    {
      auto map = table.test_flatten();
      EXPECT_EQ(6u, map->size());
      EXPECT_EQ("a", map->lookup("A"));
      EXPECT_EQ("b", map->lookup("B"));
      EXPECT_EQ("e", map->lookup("E"));
      EXPECT_EQ("f", map->lookup("F"));
      EXPECT_EQ("g", map->lookup("G"));
      EXPECT_EQ("keyG", map->lookup("key"));
    }

    // Reactivate D
    table.activateScope(ptr);
    {
      auto map = table.test_flatten();
      EXPECT_EQ(5u, map->size());
      EXPECT_EQ("a", map->lookup("A"));
      EXPECT_EQ("b", map->lookup("B"));
      EXPECT_EQ("c", map->lookup("C"));
      EXPECT_EQ("d", map->lookup("D"));
      EXPECT_EQ("keyD", map->lookup("key"));
    }

    table.activateScope(G.ptr());
    {
      auto map = table.test_flatten();
      EXPECT_EQ(6u, map->size());
      EXPECT_EQ("a", map->lookup("A"));
      EXPECT_EQ("b", map->lookup("B"));
      EXPECT_EQ("e", map->lookup("E"));
      EXPECT_EQ("f", map->lookup("F"));
      EXPECT_EQ("g", map->lookup("G"));
      EXPECT_EQ("keyG", map->lookup("key"));
    }
  }
  // This location is used to check with a debugger whether all scopes have been
  // freed.
  ptr.reset();
}

} // anonymous namespace

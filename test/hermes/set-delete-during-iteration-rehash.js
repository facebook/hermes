/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Test that Set.forEach and Map.forEach correctly visit all remaining entries
// when deleting the current entry during iteration triggers a rehash that
// compacts deleted entries. Rehash can be triggered by two code paths:
// 1) erase: shouldShrink triggers a shrink rehash
// 2) doInsert: shouldRehash triggers a grow rehash (too many deleted entries)
//
// The bug being tested: When an iterator points at a deleted entry and rehash
// compacts the data table, updateIteratorIndicesForRehash must count the deleted
// entry at the iterator's current position when adjusting the index. Using
// std::lower_bound (>=) misses it; std::upper_bound (>) correctly includes it.
//
// These numbers depend on constants in OrderedHashMap.h:
//   kInitialCapacity = 16
//   rehashThreshold = capacity >> 1  (load factor 0.5)
//   shouldShrink: keyCount <= capacity/8 && capacity > kInitialCapacity
//   shouldRehash: (keyCount + deleteCount) >= capacity/2
//   kGrowOrShrinkFactor = 2

// ============================================================================
// Tests for rehash triggered by erase (shouldShrink) — shrink rehash path
// ============================================================================

function testSetForEach() {
  print("testSetForEach");
// CHECK-LABEL: testSetForEach

  var s = new Set();

  // Add 9 entries (0-8). The 9th add triggers a grow rehash (threshold=8 for
  // capacity=16), growing capacity to 32.
  for (var i = 0; i < 9; ++i) {
    s.add(i);
  }
  // State: capacity=32, size=9, deletedCount=0
  // Data table: [0, 1, 2, 3, 4, 5, 6, 7, 8]

  // Delete entries 0-3 outside iteration.
  s.delete(0);
  s.delete(1);
  s.delete(2);
  s.delete(3);
  // State: capacity=32, size=5, deletedCount=4
  // Data table: [x, x, x, x, 4, 5, 6, 7, 8]
  // shouldShrink(32, 5) = (5 <= 4) = false, no shrink yet.

  // forEach iteration: the iterator first finds entry 4 (at data index 4).
  // The callback deletes 4, making size=4, deletedCount=5.
  // shouldShrink(32, 4) = (4 <= 4 && 32 > 16) = true, triggering rehash.
  // Rehash compacts to [5, 6, 7, 8] with capacity=16.
  //
  // The iterator was at data index 4 (a deleted entry). The deleted entry
  // indices during rehash are [0,1,2,3,4].
  //
  // With the bug (lower_bound): adjusted = 4-4 = 0, next advance starts at
  //   index 1, visiting [6,7,8] — entry 5 is skipped!
  // With the fix (upper_bound): numDeleted (5) > iteratorIndex (4), so
  //   isFirstIteration is set to true and value to 0. Next advance starts
  //   scanning at index 0, visiting [5,6,7,8] — all entries visited.

  var visited = [];
  s.forEach(function(value) {
    if (value === 4) {
      s.delete(4);
    }
    visited.push(value);
  });
  print(visited.join(","));
// CHECK-NEXT: 4,5,6,7,8
}

function testMapForEach() {
  print("testMapForEach");
// CHECK-LABEL: testMapForEach

  var m = new Map();

  // Add 9 entries to trigger grow rehash (capacity 16 -> 32).
  for (var i = 0; i < 9; ++i) {
    m.set(i, i * 10);
  }

  // Delete entries 0-3 outside iteration.
  m.delete(0);
  m.delete(1);
  m.delete(2);
  m.delete(3);

  // Same scenario as Set: deleting key 4 during forEach triggers shrink rehash.
  var visited = [];
  m.forEach(function(value, key) {
    if (key === 4) {
      m.delete(4);
    }
    visited.push(key + ":" + value);
  });
  print(visited.join(","));
// CHECK-NEXT: 4:40,5:50,6:60,7:70,8:80
}

function testSetIteratorProtocol() {
  print("testSetIteratorProtocol");
// CHECK-LABEL: testSetIteratorProtocol

  // Same setup but using the iterator protocol (for-of) instead of forEach.
  var s = new Set();
  for (var i = 0; i < 9; ++i) {
    s.add(i);
  }
  s.delete(0);
  s.delete(1);
  s.delete(2);
  s.delete(3);

  var visited = [];
  for (var value of s) {
    if (value === 4) {
      s.delete(4);
    }
    visited.push(value);
  }
  print(visited.join(","));
// CHECK-NEXT: 4,5,6,7,8
}

function testMapIteratorProtocol() {
  print("testMapIteratorProtocol");
// CHECK-LABEL: testMapIteratorProtocol

  var m = new Map();
  for (var i = 0; i < 9; ++i) {
    m.set(i, i * 10);
  }
  m.delete(0);
  m.delete(1);
  m.delete(2);
  m.delete(3);

  var visited = [];
  for (var [key, value] of m) {
    if (key === 4) {
      m.delete(4);
    }
    visited.push(key + ":" + value);
  }
  print(visited.join(","));
// CHECK-NEXT: 4:40,5:50,6:60,7:70,8:80
}

// ============================================================================
// Tests for rehash triggered by doInsert (shouldRehash) — grow rehash path
// ============================================================================

// When the current entry is deleted during iteration and then a new entry is
// inserted, doInsert may trigger a grow rehash if the accumulated deleted
// entries push (keyCount + deleteCount) past the rehash threshold. This is a
// separate code path from the shrink rehash triggered by erase above.

function testSetDeleteThenInsert() {
  print("testSetDeleteThenInsert");
// CHECK-LABEL: testSetDeleteThenInsert

  var s = new Set();

  // Add 8 entries (0-7). At capacity=16, threshold=8. Adding 8 entries does
  // NOT trigger rehash because shouldRehash is checked BEFORE insert in
  // doInsert, and at the 8th add, size is still 7.
  for (var i = 0; i < 8; ++i) {
    s.add(i);
  }
  // State: capacity=16, size=8, deletedCount=0
  // Data table: [0, 1, 2, 3, 4, 5, 6, 7]

  // forEach: first element is 0 (at data index 0).
  // Callback deletes 0, then adds 10.
  //   After delete(0): size=7, deletedCount=1.
  //     shouldShrink(16, 7) = (7 <= 2) = false, no shrink.
  //   Iterator is now at data index 0 (a deleted entry).
  //   add(10): shouldRehash(16, 7, 1) = (8 >= 8) = true! Grow rehash!
  //
  // Rehash compacts to [1,2,3,4,5,6,7] then appends 10: [1,2,3,4,5,6,7,10].
  // Capacity grows to 32. deletedEntryIndices = [0], iterator index = 0.
  //
  // With the bug (lower_bound): adjusted = 0-0 = 0, next advance starts at
  //   index 1, visiting [2,3,4,5,6,7,10] — entry 1 is skipped!
  // With the fix (upper_bound): numDeleted (1) > iteratorIndex (0), so
  //   isFirstIteration is set to true and value to 0. Next advance starts
  //   scanning at index 0, visiting [1,2,3,4,5,6,7,10].

  var visited = [];
  s.forEach(function(value) {
    if (value === 0) {
      s.delete(0);
      s.add(10);
    }
    visited.push(value);
  });
  print(visited.join(","));
// CHECK-NEXT: 0,1,2,3,4,5,6,7,10
}

function testMapDeleteThenInsert() {
  print("testMapDeleteThenInsert");
// CHECK-LABEL: testMapDeleteThenInsert

  var m = new Map();
  for (var i = 0; i < 8; ++i) {
    m.set(i, i * 10);
  }

  // Same scenario as Set: delete key 0 then insert key 10 triggers grow rehash.
  var visited = [];
  m.forEach(function(value, key) {
    if (key === 0) {
      m.delete(0);
      m.set(10, 100);
    }
    visited.push(key + ":" + value);
  });
  print(visited.join(","));
// CHECK-NEXT: 0:0,1:10,2:20,3:30,4:40,5:50,6:60,7:70,10:100
}

testSetForEach();
testMapForEach();
testSetIteratorProtocol();
testMapIteratorProtocol();
testSetDeleteThenInsert();
testMapDeleteThenInsert();
print("all tests passed");
// CHECK-NEXT: all tests passed

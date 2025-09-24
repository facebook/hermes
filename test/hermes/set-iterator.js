/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

function testRehashes() {
  print("testRehashes");
// CHECK-LABEL: testRehashes

  // These numbers depend on the constants in OrderedHashMap.h. They're chosen
  // by knowing when rehashes would happen. Since INITIAL_CAPACITY = 16 and
  // rehashThreshold() uses load factor of 0.5, this means upon construction,
  // an OrderedHashMap can store 8 elements before the first rehash happens.
  var numBeforeFirstRehash = 8;
  var numBeforeSecondRehash = 11;
  var numAfterSecondRehash = 4;

  // First add a bunch of numbers.
  var s = new Set();
  for (var i = 0; i < numBeforeFirstRehash; ++i) {
    s.add(i);
  }
  print("added first set of numbers");
// CHECK-NEXT: added first set of numbers

  // Delete some of the numbers so that the iterator below (firstDataTableIter)
  // needs to skip over and deal with them.
  s.delete(0);
  s.delete(3);
  s.delete(5);
  s.delete(6);

  // Capture an iterator at this time so we can test that it can handle two
  // rehashes later. The data table should have the following entry content in
  // insertion order:
  // deleted 1 2 deleted 4 deleted deleted 7
  var firstDataTableIter = s.keys();

  print(firstDataTableIter.next().value);
// CHECK-NEXT: 1
  print(firstDataTableIter.next().value);
// CHECK-NEXT: 2
  print(firstDataTableIter.next().value);
// CHECK-NEXT: 4

  // Now adding one more number should trigger rehash (total 9 elements).
  // firstDataTableIter should get updated to handle this rehash.
  s.add(numBeforeFirstRehash);

  // Add a bunch more numbers for testing. Note that a rehash removes the
  // deleted items, so right now the Set has 5 elements before adding any more.
  // The capacity became 32 because kGrowOrShrinkFactor = 2. The second rehash
  // will happen when the hash table has more than 16 elements.
  var begin = numBeforeFirstRehash + 1;
  var end = begin + numBeforeSecondRehash;
  for (var i = begin; i < end; ++i) {
    s.add(i);
  }

  s.delete(9);
  s.delete(11);
  s.delete(13);
  s.delete(14);
  s.delete(17);

  // Obtain iterators at this time. The current data table has the above
  // deletes and should have the following entry content in insertion order:
  // 1 2 4 7 8 deleted 10 deleted 12 deleted deleted 15 16 deleted 18 19

  // We'll use this iterator to traverse through the entire Set before adding
  // more elements.
  var allTheWayIter = s.keys();
  // We'll use this iterator to traverse through part of the Set before adding
  // more elements.
  var partOfTheWayIter = s.keys();
  // We'll use this iterator and won't traverse the Set before adding more
  // elements.
  var beginningIter = s.keys();

  // Verify that the iterator correctly skips over the deleted entries.
  var expectedNumbers = [1, 2, 4, 7, 8, 10, 12, 15, 16, 18, 19];
  expectedNumbers.forEach((num) => {
    if (allTheWayIter.next().value != num) throw new Error();
  });

  // Advance partOfTheWayIter a bit, skipping over 2 deleted entries.
  partOfTheWayIter.next();
  partOfTheWayIter.next();
  partOfTheWayIter.next();
  partOfTheWayIter.next();
  print(partOfTheWayIter.next().value);
// CHECK-NEXT: 8
  print(partOfTheWayIter.next().value);
// CHECK-NEXT: 10
  print(partOfTheWayIter.next().value);
// CHECK-NEXT: 12

  // Now we'll add more elements and cause another rehash so that the above
  // iterators will require updating.
  var begin = end;
  var end = begin + numAfterSecondRehash;
  var newNumbers = [];
  for (var i = begin; i < end; ++i) {
    s.add(i);
    newNumbers.push(i);
  }

  // allTheWayIter should continue iterating from where new numbers begin.
  newNumbers.forEach((num) => {
    if (allTheWayIter.next().value != num) throw new Error();
  });

  // partOfTheWayIter should continue where it left off and finish the first
  // group of numbers. It should then also be able to iterate through the
  // second group of numbers.
  print(partOfTheWayIter.next().value);
// CHECK-NEXT: 15
  print(partOfTheWayIter.next().value);
// CHECK-NEXT: 16
  print(partOfTheWayIter.next().value);
// CHECK-NEXT: 18
  print(partOfTheWayIter.next().value);
// CHECK-NEXT: 19
  newNumbers.forEach((num) => {
    if (partOfTheWayIter.next().value != num) throw new Error();
  });

  // beginningIter should be able to iterate through the first group of numbers
  // and then also the second group of numbers.
  expectedNumbers.forEach((num) => {
    if (beginningIter.next().value != num) throw new Error();
  });
  newNumbers.forEach((num) => {
    if (beginningIter.next().value != num) throw new Error();
  });

  // firstDataTableIter should be able to remain functional even though there
  // has been multiple rehashes. Since we already used the iterator to print 3
  // numbers, remove them from the expected list.
  expectedNumbers.shift();
  expectedNumbers.shift();
  expectedNumbers.shift();
  expectedNumbers.forEach((num) => {
    if (firstDataTableIter.next().value != num) throw new Error();
  });
  newNumbers.forEach((num) => {
    if (firstDataTableIter.next().value != num) throw new Error();
  });
}

function testClear() {
  print("testClear");
// CHECK-LABEL: testClear

  var s = new Set();
  // Add a bunch of numbers.
  for (var i = 0; i < 10; ++i) {
    s.add(i);
  }

  // Obtain some iterators at this time.
  var iter1 = s.keys();
  var iter2 = s.keys();
  print(iter1.next().value);
// CHECK-NEXT: 0
  print(iter1.next().value);
// CHECK-NEXT: 1
  print(iter2.next().value);
// CHECK-NEXT: 0
  print(iter2.next().value);
// CHECK-NEXT: 1

  // Now clear the Set while there are active iterators.
  s.clear();

  // Add a number and print it out using iterators. Iterators should handle the
  // fact that the Set was cleared.
  s.add(333);
  print(iter1.next().value);
// CHECK-NEXT: 333
  print(iter2.next().value);
// CHECK-NEXT: 333

  // Clear the Set again while there are active iterators.
  s.clear();

  // Verify again that iterators remain functional across multiple clears to the Set.
  s.add(444);
  print(iter1.next().value);
// CHECK-NEXT: 444
  print(iter2.next().value);
// CHECK-NEXT: 444
}

testRehashes();
testClear();

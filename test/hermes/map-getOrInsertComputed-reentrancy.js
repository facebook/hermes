/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Regression tests for Map.prototype.getOrInsertComputed when the callback
// re-enters getOrInsertComputed on the same map.
//
// getOrInsertComputed decides where an absent key would go, then runs the
// callback. Any structural change to the map while the callback is running --
// an insert, a delete, or a clear -- can relocate existing entries and make
// that decision wrong, so the insert has to be redone from scratch.
//
// A nested getOrInsertComputed whose callback throws is the hard part: the
// engine has to carry "the map changed" across the throw, and there are two
// independent ways to drop it. Each case below covers one of them, and both
// leave the map visibly wrong if the tracking breaks -- an entry that forEach
// visits but get() cannot find.

function checkIntegrity(m, label) {
  var visited = 0;
  var mismatches = 0;
  m.forEach(function (v, k) {
    visited++;
    if (m.get(k) !== v) {
      mismatches++;
    }
  });
  print(
    label +
      ': size=' +
      m.size +
      ' visited=' +
      visited +
      ' mismatches=' +
      mismatches,
  );
}

print('getOrInsertComputed re-entrancy');
// CHECK-LABEL: getOrInsertComputed re-entrancy

// ---------------------------------------------------------------------------
// The map is changed inside the NESTED callback, which then throws. Unwinding
// the nested call must not discard the fact that the map changed: the outer
// call still has to redo its insert.
// ---------------------------------------------------------------------------
var m1 = new Map();
var r1 = m1.getOrInsertComputed('target', function () {
  // This callback does not change the map itself; it only drives a nested
  // getOrInsertComputed whose callback changes it and then throws.
  try {
    m1.getOrInsertComputed('nested', function () {
      for (var i = 0; i < 100; i++) {
        m1.set('k' + i, i);
      }
      // Throw only after changing the map, so the change must survive the throw.
      throw new Error('boom');
    });
  } catch (e) {}
  return 'TARGET_VALUE';
});
print('nested-mutates result: ' + r1);
// CHECK-NEXT: nested-mutates result: TARGET_VALUE
print('nested-mutates get target: ' + m1.get('target'));
// CHECK-NEXT: nested-mutates get target: TARGET_VALUE
print('nested-mutates has nested: ' + m1.has('nested'));
// CHECK-NEXT: nested-mutates has nested: false
checkIntegrity(m1, 'nested-mutates');
// CHECK-NEXT: nested-mutates: size=101 visited=101 mismatches=0

// ---------------------------------------------------------------------------
// The map is changed by the OUTER callback, BEFORE it makes a nested call whose
// callback throws without changing anything. A nested call tracks changes on
// its own behalf starting from when it begins, so unwinding it must not report
// "nothing changed" back to the outer call, which was already invalidated.
// ---------------------------------------------------------------------------
var m2 = new Map();
var r2 = m2.getOrInsertComputed('target', function () {
  // Change the map FIRST, invalidating where the outer call meant to insert.
  for (var i = 0; i < 100; i++) {
    m2.set('k' + i, i);
  }
  try {
    m2.getOrInsertComputed('nested', function () {
      // Deliberately leaves the map alone: unwinding this throw is the only
      // thing that can preserve the outer call's pending invalidation.
      throw new Error('boom');
    });
  } catch (e) {}
  return 'TARGET_VALUE';
});
print('outer-mutates result: ' + r2);
// CHECK-NEXT: outer-mutates result: TARGET_VALUE
print('outer-mutates get target: ' + m2.get('target'));
// CHECK-NEXT: outer-mutates get target: TARGET_VALUE
print('outer-mutates has nested: ' + m2.has('nested'));
// CHECK-NEXT: outer-mutates has nested: false
checkIntegrity(m2, 'outer-mutates');
// CHECK-NEXT: outer-mutates: size=101 visited=101 mismatches=0

print('done');
// CHECK-NEXT: done

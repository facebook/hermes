/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -fno-std-globals --typed --dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// The index expression must match the key type.
function badKey(d: {[string]: number}): number {
  return d[0];
}

// Cannot assign to a readonly indexer.
function writeRO(o: {+[string]: number}): void {
  o["k"] = 1;
}

// Cannot read a writeonly indexer.
function readWO(o: {-[string]: number}): number {
  return o["k"];
}

// Writing requires the value type.
function badWrite(d: {[string]: number}): void {
  d["k"] = "str";
}

// Dot access uses a string key, which cannot flow into a number-keyed indexer.
function badDotKey(d: {[number]: string}): string {
  return d.k;
}

// Cannot delete from a readonly indexer (delete is a mutation).
function delRO(o: {+[string]: number}): void {
  delete o["k"];
}

// The deleted key must match the indexer's key type.
function delBadKey(d: {[string]: number}): void {
  delete d[0];
}

// Dot delete uses a string key, which cannot flow into a number-keyed indexer.
function delBadDotKey(d: {[number]: string}): void {
  delete d.k;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-indexer-member-error.js:12:12: error: ft: object index type number incompatible with index signature string
// CHECK-NEXT:  return d[0];
// CHECK-NEXT:           ^
// CHECK-NEXT:{{.*}}object-indexer-member-error.js:17:5: error: ft: cannot assign to readonly indexer
// CHECK-NEXT:  o["k"] = 1;
// CHECK-NEXT:    ^~~
// CHECK-NEXT:{{.*}}object-indexer-member-error.js:22:12: error: ft: cannot read writeonly indexer
// CHECK-NEXT:  return o["k"];
// CHECK-NEXT:           ^~~
// CHECK-NEXT:{{.*}}object-indexer-member-error.js:27:3: error: ft: incompatible assignment type: cannot implicitly cast from string to number
// CHECK-NEXT:  d["k"] = "str";
// CHECK-NEXT:  ^~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}object-indexer-member-error.js:32:12: error: ft: object index type string incompatible with index signature number
// CHECK-NEXT:  return d.k;
// CHECK-NEXT:           ^
// CHECK-NEXT:{{.*}}object-indexer-member-error.js:37:12: error: ft: cannot assign to readonly indexer
// CHECK-NEXT:  delete o["k"];
// CHECK-NEXT:           ^~~
// CHECK-NEXT:{{.*}}object-indexer-member-error.js:42:12: error: ft: object index type number incompatible with index signature string
// CHECK-NEXT:  delete d[0];
// CHECK-NEXT:           ^
// CHECK-NEXT:{{.*}}object-indexer-member-error.js:47:12: error: ft: object index type string incompatible with index signature number
// CHECK-NEXT:  delete d.k;
// CHECK-NEXT:           ^
// CHECK-NEXT:Emitted 8 errors. exiting.

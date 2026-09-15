/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function writeRO(o: {+x: number}): void {
  o.x = 1;
  o.x += 1;
  o.x++;
  --o.x;
}

function readWO(o: {-y: number}): number {
  let a: number = o.y;
  o.y += 1;
  ++o.y;
  return a;
}

// Spread of an exact object reads every source field; writeonly fields
// can't be read this way.
function spreadWO(o: {-y: number}): void {
  let a = {...o};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}variance-object-error.js:11:5: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  o.x = 1;
// CHECK-NEXT:    ^
// CHECK-NEXT:{{.*}}variance-object-error.js:12:5: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  o.x += 1;
// CHECK-NEXT:    ^
// CHECK-NEXT:{{.*}}variance-object-error.js:13:5: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  o.x++;
// CHECK-NEXT:    ^
// CHECK-NEXT:{{.*}}variance-object-error.js:14:7: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  --o.x;
// CHECK-NEXT:      ^
// CHECK-NEXT:{{.*}}variance-object-error.js:18:21: error: ft: cannot read writeonly property y
// CHECK-NEXT:  let a: number = o.y;
// CHECK-NEXT:                    ^
// CHECK-NEXT:{{.*}}variance-object-error.js:19:5: error: ft: cannot read writeonly property y
// CHECK-NEXT:  o.y += 1;
// CHECK-NEXT:    ^
// CHECK-NEXT:{{.*}}variance-object-error.js:20:7: error: ft: cannot read writeonly property y
// CHECK-NEXT:  ++o.y;
// CHECK-NEXT:      ^
// CHECK-NEXT:{{.*}}variance-object-error.js:27:12: error: ft: cannot read writeonly property y
// CHECK-NEXT:  let a = {...o};
// CHECK-NEXT:           ^~~~
// CHECK-NEXT:Emitted 8 errors. exiting.

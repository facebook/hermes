/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Each case is isolated in its own function so an annotation-phase error in one
// scope does not suppress the checks in the others.

// A default value incompatible with the destructured property type errors at a
// declaration.
function f1(): void {
  let o: {x: number} = {x: 1};
  let {x = 'str'} = o;
}

// ...and in a parameter position.
function f2({y = true}: {y: number}): void {}

// Defaults are not yet supported in typed tuple destructuring.
function f3(): void {
  let t: [number, number] = [1, 2];
  let [a, b = 9] = t;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-destr-default-error.js:19:12: error: ft: incompatible default value type
// CHECK-NEXT:  let {x = 'str'} = o;
// CHECK-NEXT:           ^~~~~
// CHECK-NEXT:{{.*}}object-destr-default-error.js:23:18: error: ft: incompatible default value type
// CHECK-NEXT:function f2({y = true}: {y: number}): void {}
// CHECK-NEXT:                 ^~~~
// CHECK-NEXT:{{.*}}object-destr-default-error.js:28:11: error: ft: default values are not yet supported in typed tuple destructuring
// CHECK-NEXT:  let [a, b = 9] = t;
// CHECK-NEXT:          ^~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.

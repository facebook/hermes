/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Error cases for fn.call(thisArg, args...).

class C {
  x: number = 0;
}
class D {
  y: string = "";
}

function withThis(this: C, n: number): number {
  return this.x + n;
}
function noThis(a: number): number {
  return a;
}

function test(): void {
  let d = new D();
  // 'this' type mismatch.
  withThis.call(d, 1);
  // Too many arguments.
  noThis.call(undefined, 1, 2);
  // Wrong arg type.
  noThis.call(undefined, "x");
  // Missing 'this'.
  noThis.call();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}builtin-function-call-error.js:29:17: error: ft: 'this' type mismatch
// CHECK-NEXT:  withThis.call(d, 1);
// CHECK-NEXT:                ^
// CHECK-NEXT:{{.*}}builtin-function-call-error.js:31:3: error: ft: function.call expects at most 1 arguments, but 2 supplied
// CHECK-NEXT:  noThis.call(undefined, 1, 2);
// CHECK-NEXT:  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}builtin-function-call-error.js:33:26: error: ft: function.call parameter 'a' type mismatch: cannot assign string to number
// CHECK-NEXT:  noThis.call(undefined, "x");
// CHECK-NEXT:                         ^~~
// CHECK-NEXT:{{.*}}builtin-function-call-error.js:35:3: error: ft: function.call requires a 'this' argument
// CHECK-NEXT:  noThis.call();
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:Emitted 4 errors. exiting.

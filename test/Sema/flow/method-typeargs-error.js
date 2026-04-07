/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Type arguments on a non-generic method.
class F {
  baz(x: number): number { return x; }
}
let f: F = new F();
f.baz<string>(42);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}method-typeargs-error.js:15:6: error: ft: type arguments provided for non-generic method
// CHECK-NEXT:f.baz<string>(42);
// CHECK-NEXT:     ^~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.

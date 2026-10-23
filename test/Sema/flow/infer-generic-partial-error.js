/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A trailing type argument that can't be inferred is reported by name.

'use strict';

function f<T, U>(t: T): T {
  return t;
}
f<number>(1);

class M {
  @Hermes.final
  m<T, U>(t: T): T {
    return t;
  }
}
new M().m<number>(1);

class D<A, B> {
  x: A;
  constructor(x: A) {
    this.x = x;
  }
}
new D<number>(5);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}infer-generic-partial-error.js:17:2: error: ft: could not infer type argument for type parameter 'U'
// CHECK-NEXT:f<number>(1);
// CHECK-NEXT: ^~~~~~~~
// CHECK-NEXT:{{.*}}infer-generic-partial-error.js:25:10: error: ft: could not infer type argument for type parameter 'U'
// CHECK-NEXT:new M().m<number>(1);
// CHECK-NEXT:         ^~~~~~~~
// CHECK-NEXT:{{.*}}infer-generic-partial-error.js:33:6: error: ft: could not infer type argument for type parameter 'B'
// CHECK-NEXT:new D<number>(5);
// CHECK-NEXT:     ^~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.

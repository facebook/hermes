/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-sema -fno-std-globals 2>&1) | %FileCheck --match-full-lines %s

class A {
  set #x(v) {}
  get #y() {}
  static m1(o) {
    sink(o.#x);
// CHECK:{{.*}}private-load-store-error.js:14:10: error: Cannot load from a private name that only defines a setter.
// CHECK-NEXT:    sink(o.#x);
// CHECK-NEXT:         ^~~~
    o.#y = 12;
// CHECK:{{.*}}private-load-store-error.js:18:5: error: Cannot store to a private name that only defines a getter.
// CHECK-NEXT:    o.#y = 12;
// CHECK-NEXT:    ^~~~~~~~~
    sink(o?.#x);
// CHECK:{{.*}}private-load-store-error.js:22:10: error: Cannot load from a private name that only defines a setter.
// CHECK-NEXT:    sink(o?.#x);
// CHECK-NEXT:         ^~~~~
    o?.#y = 12;
// CHECK:{{.*}}private-load-store-error.js:26:5: error: Cannot store to a private name that only defines a getter.
// CHECK-NEXT:    o?.#y = 12;
// CHECK-NEXT:    ^~~~~~~~~~
  }
}

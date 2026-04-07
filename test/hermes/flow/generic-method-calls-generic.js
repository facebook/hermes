/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class Caller {
  x: number;

  constructor() {
    this.x = 0;
  }

  @Hermes.final
  wrap<T>(val: T): T {
    return val;
  }

  @Hermes.final
  doubleWrap<T>(val: T): T {
    // A generic method calling another generic method on 'this'.
    return this.wrap<T>(val);
  }

  @Hermes.final
  tripleWrap<T>(val: T): T {
    return this.doubleWrap<T>(val);
  }
}

let c: Caller = new Caller();

print(c.doubleWrap<number>(42));
// CHECK: 42
print(c.doubleWrap<string>("nested"));
// CHECK-NEXT: nested

print(c.tripleWrap<number>(99));
// CHECK-NEXT: 99
print(c.tripleWrap<string>("deep"));
// CHECK-NEXT: deep

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// @Hermes.final on a non-generic method.
class Counter {
  count: number;

  constructor() {
    this.count = 0;
  }

  @Hermes.final
  inc(): number {
    this.count = this.count + 1;
    return this.count;
  }

  @Hermes.final
  get(): number {
    return this.count;
  }
}

let ctr: Counter = new Counter();
print(ctr.inc());
// CHECK: 1
print(ctr.inc());
// CHECK-NEXT: 2
print(ctr.get());
// CHECK-NEXT: 2

// Non-generic final alongside a non-final method.
class Obj {
  val: number;

  constructor() {
    this.val = 7;
  }

  @Hermes.final
  finalGet(): number {
    return this.val;
  }

  normalGet(): number {
    return this.val;
  }
}

let obj: Obj = new Obj();
print(obj.finalGet());
// CHECK-NEXT: 7
print(obj.normalGet());
// CHECK-NEXT: 7

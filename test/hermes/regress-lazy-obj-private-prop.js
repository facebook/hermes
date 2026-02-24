/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s
// RUN: %shermes -exec %s | %FileCheck %s

// Test that assigning a private field to the lazy object f0 works.
function lazyFun() {}

class C0 {
  constructor() {
    return lazyFun;
  }
}
class C1 extends C0 {
  #p = 5;
  static printP(obj) { print(#p in obj, obj.#p); }
}
// Assign #p to lazyFun.
new C1();

// Do a GC to make sure the heap is well formed.
gc();

C1.printP(lazyFun);
//CHECK: true 5

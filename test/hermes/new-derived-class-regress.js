/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

// Fix a regression where GetConstructedObjectInst produced a non-object
// type, which violates its invariant. This was happening because the
// Construct call which produced the GetConstructedObjectInst was known to
// fail, which would guarantee that the non-object producing case would
// never execute. However, with CodeMotion, it can move the
// GetConstructedObjectInst before the call, which would then break the
// invariant.

print("derived class constructor regression");
//CHECK-LABEL: derived class constructor regression

class f3 {}
for (let v1 = 0; v1 < 1; v1++) {
  class C4 extends f3 {
    constructor() {
      return null;
    }
  }
  try {
    new C4(v1);
  } catch (e) {
    print(e.name);
//CHECK-NEXT: TypeError
  }
}

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xenable-tdz -test262 %s | %FileCheck --match-full-lines %s

// Test a TDZ regression, where UnionNarrowTrustedInst was lifted
// incorrectly. From IRGen, there is a ThrowIfInst emitted, followed by a
// UnionNarrowTrustedInst. The UnionNarrowTrustedInst is only vaild in the
// non-throwing case of ThrowIf, but if the UnionNarrowTrustedInst is lifted
// by CodeMotion, it would execute before the verifying instruction of
// ThrowIf. This invalid execution results in an invariant being violated.

print("UnionNarrowTrustedInst regression");
//CHECK-LABEL: UnionNarrowTrustedInst regression

function main() {
  for(;;) {
    const foo = print(foo, typeof foo)
  }
}
try {
  main();
} catch (e) {
  print(e.name);
//CHECK-NEXT: ReferenceError
}

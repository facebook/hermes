/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Test typed array will check for NaN when reading from an index.

print("typed-array-at-nan");
//CHECK-LABEL: typed-array-at-nan
let ab = new ArrayBuffer(8);
let i32a = new Int32Array(ab);

i32a[0] = 0xcafebabe;
i32a[1] = 0xffff0000;

let f32a = new Float64Array(ab);
var shouldBeNan = f32a.at(0)
print(shouldBeNan);
// CHECK: NaN

/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("BEGIN");
//CHECK: BEGIN

var {a, c, ...rest} = {x: 10, y: 20, c: 5, a: 3, z: 30}
print(a, c, Object.entries(rest));
//CHECK-NEXT: 3 5 x,10,y,20,z,30

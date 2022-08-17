/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var x = 5;
print(-x);
//CHECK: -5

x = -5.5;
print(-x);
//CHECK: 5.5

x = 4e30;
print(-x);
//CHECK: -4e+30

x = 0;
print(x);
print(-x);
print(1/x);
print(1/-x);
//CHECK: 0
//CHECK: 0
//CHECK: Infinity
//CHECK: -Infinity

x = "1";
print(-x);
//CHECK: -1


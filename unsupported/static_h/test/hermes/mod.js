/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print(10 % 2);
//CHECK: 0

print(10 % 3);
//CHECK: 1

print(10 % 0);
//CHECK: NaN

print(10 % Infinity);
//CHECK: 10

print(10 % NaN);
//CHECK: NaN

print(0 % 10);
//CHECK: 0

print(Infinity % 10);
//CHECK: NaN

print(NaN % 10);
//CHECK: NaN

print(5.5 % 1.5);
//CHECK: 1

print(5.5 % -1.5);
//CHECK: 1

print(-5.5 % 1.5);
//CHECK: -1

print(-5.5 % -1.5);
//CHECK: -1

print(5.5 % 2.5);
//CHECK: 0.5

print(5.5 % -2.5);
//CHECK: 0.5

print(-5.5 % 2.5);
//CHECK: -0.5

print(-5.5 % -2.5);
//CHECK: -0.5


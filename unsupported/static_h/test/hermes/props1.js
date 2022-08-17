/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Read-only properties in the prototype cannot be overridden in the child.
x = {}
p = x.__proto__;
Object.defineProperty(p, "a", {writable:false, value:10});
print(x.a);
//CHECK: 10
x.a = 20;
print(x.a);
//CHECK-NEXT: 10

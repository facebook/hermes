/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

var {...rest} = {x: 10, y: 20};
print(Object.entries(rest));
//CHECK-NEXT: x,10,y,20

// These two tests for duplicate identifiers were motivated by the use of
// 'copyDataProperties' internally. This process creates a dummy object with
// properties to ignore when creating the "rest" object in destructuring. This
// must not be created with duplicate properties or an assertion may be thrown.
var {a, a, ...b} = {a: 42, c: 69};
print(a, Object.entries(b));
//CHECK-NEXT: 42 c,69

var {['a']: b, ['a']: b, ...c} = {a: 43, d: 70};
print(b, Object.entries(c));
//CHECK-NEXT: 43 d,70

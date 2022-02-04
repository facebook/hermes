/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var obj = {a: 'a', b: 'b', '1': '1'};
print(obj.a, obj.b, obj[1]);
//CHECK: a b 1

delete obj.a;
print(obj.a, obj.b, obj[1]);
//CHECK: undefined b 1

delete obj['b'];
print(obj.a, obj.b, obj[1]);
//CHECK: undefined undefined 1

delete obj[1];
print(obj.a, obj.b, obj[1]);
//CHECK: undefined undefined undefined

var x = 0;
print(delete x.a);
//CHECK: true

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec -typed %s | %FileCheck --match-full-lines %s

var obj: any = {a: 'a', b: 'b', '1': '1'};
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

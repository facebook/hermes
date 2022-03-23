/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

function type_of(x) { return typeof(x); }
function type_of_str(x) { return typeof(" " + x); }
function type_of_number(x) { return typeof(x + 1); }
function type_of_object(x) { return typeof({"x" : x }); }

print(type_of(5));
//CHECK: number

print(type_of(5.5));
//CHECK-NEXT: number

print(type_of(NaN));
//CHECK-NEXT: number

print(type_of(undefined));
//CHECK-NEXT: undefined

print(type_of(null));
//CHECK-NEXT: object

print(type_of(true));
//CHECK-NEXT: boolean

print(type_of(false));
//CHECK-NEXT: boolean

print(type_of("123"));
//CHECK-NEXT: string

print(type_of([]));
//CHECK-NEXT: object

print(type_of({}));
//CHECK-NEXT: object

print(type_of(function () {}));
//CHECK-NEXT: function

print(type_of(Object.defineProperties));
//CHECK-NEXT: function

print(type_of_str(""));
//CHECK-NEXT: string

print(type_of_number(4));
//CHECK-NEXT: number

print(type_of_object(4));
//CHECK-NEXT: object

print(typeof((function () { return this; })()));
//CHECK-NEXT: object

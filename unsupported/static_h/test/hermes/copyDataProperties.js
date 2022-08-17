/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods -O %s | %FileCheck --match-full-lines %s

print("START");
// CHECK: START

var o = {a: 10, b: 20, c: 30};
var x = {b:0, d:1}

var res = HermesInternal.copyDataProperties({}, o);
print("res=", Object.entries(res))
//CHECK-NEXT: res= a,10,b,20,c,30

var res = HermesInternal.copyDataProperties({}, o, x);
print("res=", Object.entries(res))
//CHECK-NEXT: res= a,10,c,30

var res = HermesInternal.copyDataProperties({}, o, o);
print("res=", Object.entries(res))
//CHECK-NEXT: res=

var res = HermesInternal.copyDataProperties({}, x, o);
print("res=", Object.entries(res))
//CHECK-NEXT: res= d,1

var o = {1: 11, 2: 12, 4: 14};
var x = [100, 101, , 103];

var res = HermesInternal.copyDataProperties({}, o);
print("res=", Object.entries(res))
//CHECK-NEXT: res= 1,11,2,12,4,14
var res = HermesInternal.copyDataProperties({}, o, o);
print("res=", Object.entries(res))
//CHECK-NEXT: res=

var res = HermesInternal.copyDataProperties({}, x);
print("res=", Object.entries(res))
//CHECK-NEXT: res= 0,100,1,101,3,103
var res = HermesInternal.copyDataProperties({}, x, x);
print("res=", Object.entries(res))
//CHECK-NEXT: res=

var res = HermesInternal.copyDataProperties({}, x, o);
print("res=", Object.entries(res))
//CHECK-NEXT: res= 0,100,3,103
var res = HermesInternal.copyDataProperties({}, o, x);
print("res=", Object.entries(res))
//CHECK-NEXT: res= 2,12,4,14

var res = HermesInternal.copyDataProperties({a:10, b:20, c:30}, {b:21, c:31, d:41}, {c:0})
print("res=", Object.entries(res))
//CHECK-NEXT: res= a,10,b,21,c,30,d,41

var t = {}
var res = HermesInternal.copyDataProperties(t, null);
print(t === res);
//CHECK-NEXT: true
var res = HermesInternal.copyDataProperties(t, undefined);
print(t === res);
//CHECK-NEXT: true
var res = HermesInternal.copyDataProperties(t, "Hello");
print("res=", Object.entries(res))
//CHECK-NEXT: res= 0,H,1,e,2,l,3,l,4,o

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -strict -Wno-undefined-variable -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -strict -Wno-undefined-variable -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

"use strict";

if (typeof print === "undefined")
    var print = console.log;

function mustThrow(f) {
    try {
        f();
    } catch (e) {
        print("caught", e.name, e.message);
        return;
    }
    print("DID NOT THROW");
}

print("string-props");
//CHECK-LABEL: string-props
var s = new String("abc");
var d = Object.getOwnPropertyDescriptor(s, 1);
print(d.value, d.writable, d.enumerable, d.configurable);
//CHECK-NEXT: b false true false

print(""+Object.defineProperty(s, 2, {writable:false}));
//CHECK-NEXT: abc
print(""+Object.defineProperty(s, 2, {value:"c"}));
//CHECK-NEXT: abc

print("array-props");
//CHECK-LABEL: array-props

var a = ["a", "b", "c"];
var d = Object.getOwnPropertyDescriptor(a, 1);
print(d.value, d.writable, d.enumerable, d.configurable);
//CHECK-NEXT: b true true true

Object.freeze(a);
mustThrow(function() { a[1] = 5;});
//CHECK-NEXT: caught TypeError {{.*}}

var d = Object.getOwnPropertyDescriptor(a, 1);
print(d.value, d.writable, d.enumerable, d.configurable);
//CHECK-NEXT: b false true false

print("array-index-like-props");
//CHECK-LABEL: array-index-like-props

var a = ["a", "b", "c"];

Object.defineProperty(a, 0, {enumerable:false});
var d = Object.getOwnPropertyDescriptor(a, 0);
print(d.value, d.writable, d.enumerable, d.configurable);
//CHECK-NEXT: a true false true

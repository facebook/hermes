/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var saveProto = Object.prototype;
var singleton = new Object();

Object = function Foo() {
    print("Foo");
    return singleton;
}

print("start of test");
//CHECK: start of test
var x = {};
print(x.toString());
//CHECK-NEXT: [object Object]
print(x !== singleton);
//CHECK-NEXT: true
print(x.__proto__ === saveProto);
//CHECK-NEXT: true

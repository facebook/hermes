/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O  %s -fno-inline | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s             | %FileCheck --match-full-lines %s

function init() {
    print("init");
    return {
        get "a"() {
            print("get a()");
            return "this is a";
        },
        get b() {
            print("get b()");
            return "this is b";
        },
        get c() {
            print("get c()");
            return "this is c";
        },
    };
}

function keyA() {
    print("keyA")
    return "a";
}

function keyB() {
    print("keyB")
    return "b";
}

var valObj = {};
function val() {
    print("val")
    return valObj;
}

({[keyA()]: val().a, c: val().c, [keyB()]: val().b} = init());
// CHECK: init
// CHECK-NEXT: keyA
// CHECK-NEXT: val
// CHECK-NEXT: get a()
// CHECK-NEXT: val
// CHECK-NEXT: get c()
// CHECK-NEXT: keyB
// CHECK-NEXT: val
// CHECK-NEXT: get b()

print(JSON.stringify(valObj));
// CHECK-NEXT: {"a":"this is a","c":"this is c","b":"this is b"}

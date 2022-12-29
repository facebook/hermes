/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fcache-new-object -exec %s | %FileCheck --match-full-lines %s

function fun1(){
    this.x = 1;
    this.y = 2;
    this.z = 3;
}

fun1.prototype = { set y(v){ print("setter invoked"); } };
var obj = new fun1();
// CHECK-LABEL: setter invoked
print(JSON.stringify(obj));
// CHECK-NEXT: {"x":1,"z":3}

function fun2(){
    this.x = 1;
    this.y = 2;
    this.z = 3;
}
// Create a legitimate cache entry.
obj = new fun2();
fun2.prototype = { set y(v){ print("setter invoked"); } };
// Verify that it is invalidted.
obj = new fun2();
// CHECK-NEXT: setter invoked

function fun3(){
    this.x = 1;
    this.y = 2;
    this.z = 3;
}
fun3.prototype = new Proxy({}, {set(){ print("trap invoked"); }});
// Verify that the trap is invoked
obj = new fun3();
// CHECK-NEXT: trap invoked
// CHECK-NEXT: trap invoked
// CHECK-NEXT: trap invoked

function fun4(){
    this.x = 1;
    this.y = 2;
    this.z = 3;
}
fun4.prototype = {a : 1};
// Put the prototype in dictionary mode.
delete fun4.prototype.a;
obj = new fun4();
// Modify the dictionary mode prototype to contain a setter.
Object.defineProperty(fun4.prototype, "x", {set(){ print("setter invoked");} });
obj = new fun4();
// CHECK-NEXT: setter invoked

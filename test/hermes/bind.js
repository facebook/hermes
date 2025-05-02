/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=C.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: LC_ALL=C.UTF-8 %shermes -exec %s | %FileCheck --match-full-lines %s

"use strict";

function foo(a, b) {
    print("this="+this, "args.length="+arguments.length, "a="+a, "b="+b);
}

foo.bind(0, 1, 2)();
//CHECK:this=0 args.length=2 a=1 b=2
foo.bind("thisValue", 10, 20, 30)();
//CHECK-NEXT:this=thisValue args.length=3 a=10 b=20
foo.bind(1, 11, 21, 31)(41, 51);
//CHECK-NEXT:this=1 args.length=5 a=11 b=21
foo.bind("this", 1)();
//CHECK-NEXT:this=this args.length=1 a=1 b=undefined
foo.bind("this", 1)(2, 3);
//CHECK-NEXT:this=this args.length=3 a=1 b=2
foo.bind()();
//CHECK-NEXT:this=undefined args.length=0 a=undefined b=undefined
foo.bind()(1);
//CHECK-NEXT:this=undefined args.length=1 a=1 b=undefined
print(foo.bind().name);
//CHECK-NEXT: bound foo
print(foo.bind().bind().name);
//CHECK-NEXT: bound bound foo
var unicodeFun = function føø() {}
print(unicodeFun.bind().name);
//CHECK-NEXT: bound føø

var bound = foo.bind("this", 1);
print(bound.prototype, bound.length);
//CHECK-NEXT: undefined 1
var bound = foo.bind("this", 1, 2, 3);
print(bound.prototype, bound.length);
//CHECK-NEXT: undefined 0

var func = Object.defineProperty(function() {}, 'name', {value: 3});
print('"' + func.bind().name + '"');
// CHECK-NEXT: "bound "

// Check bound constructors.
function MyClass(prop2, prop3) {
    this.prop2 = prop2;
    this.prop3 = prop3;
}
MyClass.prototype = {prop1: "prop1"};

var BoundClass = MyClass.bind("unused", "prop2");
print(BoundClass.prototype, BoundClass.length);
//CHECK-NEXT: undefined 1

var x = new BoundClass("prop3");
print(x.__proto__ == MyClass.prototype, x.prop1, x.prop2, x.prop3);
//CHECK-NEXT: true prop1 prop2 prop3

print("override-length");
//CHECK-LABEL: override-length
print(foo.length)
//CHECK-NEXT: 2

Object.defineProperty(foo, "length", {get: function() { return "aa"; }});
print(foo.length)
//CHECK-NEXT: aa

print(foo.bind(null).length);
//CHECK-NEXT: 0

Object.defineProperty(foo, "length", {get: function() { throw TypeError("HA!"); }});
try {
    foo.bind(null);
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught TypeError HA!

Object.defineProperty(foo, "length", {value: {valueOf: function() {throw TypeError("HAHA!");}}});
print(foo.bind(null).length);
//CHECK-NEXT: 0

// new.target is set to target function, not bound function.
function bar() {
  print(new.target === bar);
}
var barBound = bar.bind();
new barBound();
//CHECK-NEXT: true

// new.target is setup correctly in nested bound function calls.
(function (){
  function a() {
    print(new.target === a);
  }
  var boundA = a.bind();
  var boundA2 = boundA.bind();
  let o = Reflect.construct(boundA2, [], boundA);
//CHECK-NEXT: true
  print(Object.getPrototypeOf(o).constructor === a);
//CHECK-NEXT: true
  new boundA2();
//CHECK-NEXT: true
})();


function baz() {}

Object.setPrototypeOf(baz, foo);
var bazBound = baz.bind(null);
print(Object.getPrototypeOf(bazBound) === Object.getPrototypeOf(baz));
//CHECK-NEXT: true

Object.setPrototypeOf(baz, null);
var bazBound = Function.prototype.bind.apply(baz, bar);
print(Object.getPrototypeOf(bazBound));
//CHECK-NEXT: null

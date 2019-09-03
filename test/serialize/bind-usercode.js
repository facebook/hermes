// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

"use strict";

function foo(a, b) {
    print("this="+this, "args.length="+arguments.length, "a="+a, "b="+b);
}

var bound = foo.bind(0, 1, 2)
var bound1 = foo.bind("thisValue", 10, 20, 30)
var bound2 = foo.bind(1, 11, 21, 31);
var bound3 = foo.bind("this", 1);
var bound4 = foo.bind("this", 1)
var bound5 = foo.bind();

var bound6 = foo.bind("this", 1);
var bound7 = foo.bind("this", 1, 2, 3);

var func = Object.defineProperty(function() {}, 'name', {value: 3});
var bound8 = func.bind()

function MyClass(prop2, prop3) {
    this.prop2 = prop2;
    this.prop3 = prop3;
}
MyClass.prototype = {prop1: "prop1"};

var BoundClass = MyClass.bind("unused", "prop2");

var x = new BoundClass("prop3");

serializeVM(function(){
  bound();
  //CHECK:this=0 args.length=2 a=1 b=2
  bound1();
  //CHECK-NEXT:this=thisValue args.length=3 a=10 b=20
  bound2(41, 51)
  //CHECK-NEXT:this=1 args.length=5 a=11 b=21
  bound3()
  //CHECK-NEXT:this=this args.length=1 a=1 b=undefined
  bound4(2, 3);
  //CHECK-NEXT:this=this args.length=3 a=1 b=2
  bound5();
  //CHECK-NEXT:this=undefined args.length=0 a=undefined b=undefined
  bound5(1);
  //CHECK-NEXT:this=undefined args.length=1 a=1 b=undefined
  print(bound5.name);
  //CHECK-NEXT: bound foo
  print(bound5.bind().name);
  //CHECK-NEXT: bound bound foo

  print(bound6.prototype, bound6.length);
  //CHECK-NEXT: undefined 1
  print(bound7.prototype, bound7.length);
  //CHECK-NEXT: undefined 0

  print('"' + bound8.name + '"');
  // CHECK-NEXT: "bound "

  // Check bound constructors.
  print(BoundClass.prototype, BoundClass.length);
  //CHECK-NEXT: undefined 1

  print(x.__proto__ == MyClass.prototype, x.prop1, x.prop2, x.prop3);
  //CHECK-NEXT: true prop1 prop2 prop3
})

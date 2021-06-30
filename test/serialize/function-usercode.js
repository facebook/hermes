/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict"

function foo() {}
var F = function(x) { return x; };
var f = new F(1);
var F2 = function(a,b,c,d,e,f,g,h,i,j,k){}
var foo = function badlength(a, b, c){};
Object.defineProperty(foo, "length", {value:"aa"});
var f2 = function(a, b) {
  print(arguments.length, this, a, b);
};
f2.call()
var f3 = function(a, b) {
  print(arguments.length, this, a, b);
};
var args = ['a',,];
args.__proto__[1] = 'b';
var f4 = Function('a,b', 'var r = this+a+b; return r')
var f5 = Function('a,b', 'if (a) {return b} else {return 0}')
var f6 = Function()
var f7 = Function('x', 'return x // comment')

serializeVM(function() {
  print("foo.length/configurable:", Object.getOwnPropertyDescriptor(foo, "length").configurable);
  //CHECK: foo.length/configurable: true
  print("foo.__proto__.length/configurable:", Object.getOwnPropertyDescriptor(foo.__proto__, "length").configurable);
  //CHECK: foo.__proto__.length/configurable: true
  print("Function.length/configurable:", Object.getOwnPropertyDescriptor(Function, "length").configurable);
  //CHECK: Function.length/configurable: true

  print('Function');
  // CHECK-LABEL: Function

  F.prototype.prop1 = 'hermes';
  print(F(184));
  // CHECK-NEXT: 184
  print(F.prototype);
  // CHECK-NEXT: [object Object]
  print(F.length);
  // CHECK-NEXT: 1
  print(f.__proto__ === F.prototype, f.constructor === F, f.prop1);
  // CHECK-NEXT: true true hermes

  print('toString');
  // CHECK-LABEL: toString
  print(F2);
  // CHECK-NEXT: function F2(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) { [bytecode] }

  // Non-string .length
  print(foo);
  // CHECK-NEXT: function badlength() { [bytecode] }

  print('call');
  // CHECK-LABEL: call
  print(f2.length);
  // CHECK-NEXT: 2
  f2.call(1, 2, 3);
  // CHECK-NEXT: 2 1 2 3
  f2.call();
  // CHECK-NEXT: 0 undefined undefined undefined
  f2.call(1, 2, 3, 4, 5, 6);
  // CHECK-NEXT: 5 1 2 3

  print('apply');
  // CHECK-LABEL: apply
  print(f3.length);
  // CHECK-NEXT: 2
  f3.apply(1, [2, 3]);
  // CHECK-NEXT: 2 1 2 3
  f3.apply();
  // CHECK-NEXT: 0 undefined undefined undefined
  f3.apply(1);
  // CHECK-NEXT: 0 1 undefined undefined
  f3.apply(1, []);
  // CHECK-NEXT: 0 1 undefined undefined
  f3.apply(1, [2, 3, 4, 5, 6]);
  // CHECK-NEXT: 5 1 2 3
  f3.apply(1, {0: 'a', 1: 'b', length: 2});
  // CHECK-NEXT: 2 1 a b
  f3.apply(1, args);
  // CHECK-NEXT: 2 1 a b

  print((f4).call(3, 5, 7));
  // CHECK-NEXT: 15
  print((f5)(true, 3));
  // CHECK-NEXT: 3
  print(f6.name);
  // CHECK-NEXT: anonymous

  // Line comment at the end of the body
  print(f7(1));
  // CHECK-NEXT: 1
})

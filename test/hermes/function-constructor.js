/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s
"use strict";

print('Function');
// CHECK-LABEL: Function
print(String.length);
// CHECK-NEXT: 1
print(typeof Function.__proto__, Function.__proto__ === Function.prototype);
// CHECK-NEXT: function true
print(Function.__proto__.length);
// CHECK-NEXT: 0
print(Function.length);
// CHECK-NEXT: 1
print(typeof print.constructor);
// CHECK-NEXT: function
print(print.length);
// CHECK-NEXT: 1
print(typeof print.__proto__);
// CHECK-NEXT: function
print(Function.__proto__());
// CHECK-NEXT: undefined
try {
  new Function.__proto__();
} catch (e) {
  print(e);
}
// CHECK-NEXT: TypeError: This function cannot be used as a constructor.

var F = function(x) { return x; };
F.prototype.prop1 = 'hermes';
print(F(184));
// CHECK-NEXT: 184
print(F.prototype);
// CHECK-NEXT: [object Object]
print(F.length);
// CHECK-NEXT: 1
var f = new F(1);
print(f.__proto__ === F.prototype, f.constructor === F, f.prop1);
// CHECK-NEXT: true true hermes

print('toString');
// CHECK-LABEL: toString
print(function (){});
// CHECK-NEXT: function () { [bytecode] }
print(function foo(){});
// CHECK-NEXT: function foo() { [bytecode] }
print(function(a,b,c){});
// CHECK-NEXT: function (a0, a1, a2) { [bytecode] }
print(function(a,b,c,d,e,f,g,h,i,j,k){});
// CHECK-NEXT: function (a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) { [bytecode] }
print(Map.toString());
// CHECK-NEXT: function Map() { [native code] }

// Non-string .length
var foo = function badlength(a, b, c){};
Object.defineProperty(foo, "length", {value:"aa"});
print(foo);
// CHECK-NEXT: function badlength() { [bytecode] }

print('call');
// CHECK-LABEL: call
var f = function(a, b) {
  print(arguments.length, this, a, b);
};
print(f.length);
// CHECK-NEXT: 2
f.call(1, 2, 3);
// CHECK-NEXT: 2 1 2 3
f.call();
// CHECK-NEXT: 0 undefined undefined undefined
f.call(1, 2, 3, 4, 5, 6);
// CHECK-NEXT: 5 1 2 3

print('apply');
// CHECK-LABEL: apply
var f = function(a, b) {
  print(arguments.length, this, a, b);
};
print(f.length);
// CHECK-NEXT: 2
f.apply(1, [2, 3]);
// CHECK-NEXT: 2 1 2 3
f.apply();
// CHECK-NEXT: 0 undefined undefined undefined
f.apply(1);
// CHECK-NEXT: 0 1 undefined undefined
f.apply(1, []);
// CHECK-NEXT: 0 1 undefined undefined
f.apply(1, [2, 3, 4, 5, 6]);
// CHECK-NEXT: 5 1 2 3
f.apply(1, {0: 'a', 1: 'b', length: 2});
// CHECK-NEXT: 2 1 a b
var args = ['a',,];
args.__proto__[1] = 'b';
f.apply(1, args);
// CHECK-NEXT: 2 1 a b

var f = function() {};
print(f.length);
// CHECK-NEXT: 0
var f = eval('(function(){return "hello"})');
print(f());
// CHECK-NEXT: hello
print(typeof(Function()), (Function())());
// CHECK-NEXT: function undefined
print((Function('return "hello"'))());
// CHECK-NEXT: hello
print((Function('a', 'return a'))(1));
// CHECK-NEXT: 1
print((Function('a', 'b', 'return a+b'))(1, 2));
// CHECK-NEXT: 3
print((Function('a,b', 'return a+b'))(1, 2));
// CHECK-NEXT: 3
print((Function('a,b', 'c', 'return [a,b,c]'))(1, 2, 3));
// CHECK-NEXT: 1,2,3
print((Function('a,b', 'return this + a + b')).call(3, 5, 7));
// CHECK-NEXT: 15
print((Function('a,b', 'var r = this+a+b; return r')).call(3, 5, 7));
// CHECK-NEXT: 15
print((Function('a,b', 'if (a) {return b} else {return 0}'))(true, 3));
// CHECK-NEXT: 3
print((Function('a,b', 'if (a) {return b} else {return 0}'))(false, 3));
// CHECK-NEXT: 0
print(Function("return this").length);
// CHECK-NEXT: 0
print(Function("return this")());
// CHECK-NEXT: [object global]
print(Function("return this;")());
// CHECK-NEXT: [object global]
print(Function("return\nthis")());
// CHECK-NEXT: undefined
print(Function("return this").__proto__ === Function.prototype);
// CHECK-NEXT: true
try {Function('a)', 'print(1);')} catch (e) {print('caught', e.name, e.message)}
// CHECK-NEXT: caught SyntaxError {{.*}}
try {Function('});print(1);({')} catch (e) {print('caught', e.name, e.message)}
// CHECK-NEXT: caught SyntaxError {{.*}}
print(Function().name);
// CHECK-NEXT: anonymous

// Line comment at the end of the body
print(Function('x', 'return x // comment')(1));
// CHECK-NEXT: 1

// Hashbang comments are not supported in function bodies
try {Function('#! comment')} catch (e) {print('caught', e.name, e.message)}
// CHECK-NEXT: caught SyntaxError {{.*}}

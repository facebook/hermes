/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

// Testing the chart in ES6.0 25.2.
// Make sure Generators and the functions which surround them have the proper
// inheritance structure.

function *f() {}
var Generator = Object.getPrototypeOf(f);
var GeneratorFunction = Generator.constructor;

print('generator object model');
// CHECK-LABEL: generator object model

print(Generator[Symbol.toStringTag]);
// CHECK-NEXT: GeneratorFunction
print(Generator.prototype[Symbol.toStringTag]);
// CHECK-NEXT: Generator

print(1, GeneratorFunction.prototype === Generator);
// CHECK-NEXT: 1 true
print(2, Generator.prototype === Object.getPrototypeOf(f.prototype));
// CHECK-NEXT: 2 true
print(3, f instanceof GeneratorFunction);
// CHECK-NEXT: 3 true
print(4, Object.getPrototypeOf(Generator) === Function.prototype);
// CHECK-NEXT: 4 true
print(5, GeneratorFunction.__proto__ === Function);
// CHECK-NEXT: 5 true

var instance = GeneratorFunction();
print(5, typeof instance.prototype);
// CHECK-NEXT: 5 object
print(6,
  Object.getPrototypeOf(instance.prototype) ===
  Object.getPrototypeOf(instance).prototype);
// CHECK-NEXT: 6 true

var GeneratorPrototype = Object.getPrototypeOf(
  Object.getPrototypeOf(f())
);
print(GeneratorPrototype[Symbol.toStringTag]);
// CHECK-NEXT: Generator

// If .prototype is null, fall back to generator prototype.
function *g() {}
g.prototype = null;
print(Object.getPrototypeOf(g()) === Generator.prototype);
// CHECK-NEXT: true

// f.prototype should not have a .constructor property.
print(Object.getOwnPropertyNames(f.prototype).length);
// CHECK-NEXT: 0

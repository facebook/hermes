/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// prohibitInvoke is enforced by every backend -- the bytecode interpreter,
// serialized bytecode, and the Static Hermes AOT (shermes) backend -- via a
// check in the callee prologue. A constructor-only function (an ES6 class
// constructor) called without `new`, and a call-only function (an arrow,
// method, generator, or async function) invoked with `new`, both throw a
// TypeError.

// An ES6 class constructor must not be callable without `new`.
print('=== class called without new ===');
//CHECK-LABEL: === class called without new ===
class C {
  constructor() {
    print('CTOR BODY RAN');
  }
}
try {
  C();
  print('no-throw');
} catch (e) {
  print('threw: ' + e.message);
}
//CHECK-NEXT: threw: Class constructor invoked without new

// An arrow function must not be usable as a constructor.
print('=== new arrow ===');
//CHECK-LABEL: === new arrow ===
var arrow = () => {
  print('ARROW BODY RAN');
};
try {
  new arrow();
  print('no-throw');
} catch (e) {
  print('threw: ' + e.message);
}
//CHECK-NEXT: threw: Function is not a constructor

// An object method must not be usable as a constructor.
print('=== new method ===');
//CHECK-LABEL: === new method ===
var obj = {
  m() {
    print('METHOD BODY RAN');
  },
};
try {
  new obj.m();
  print('no-throw');
} catch (e) {
  print('threw: ' + e.message);
}
//CHECK-NEXT: threw: Function is not a constructor

// A generator function must not be usable as a constructor.
print('=== new generator ===');
//CHECK-LABEL: === new generator ===
function* g() {
  yield 1;
}
try {
  new g();
  print('no-throw');
} catch (e) {
  print('threw: ' + e.message);
}
//CHECK-NEXT: threw: Function is not a constructor

// An async function must not be usable as a constructor.
print('=== new async function ===');
//CHECK-LABEL: === new async function ===
async function af() {
  print('ASYNC BODY RAN');
}
try {
  new af();
  print('no-throw');
} catch (e) {
  print('threw: ' + e.message);
}
//CHECK-NEXT: threw: Function is not a constructor

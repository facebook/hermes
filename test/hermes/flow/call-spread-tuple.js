/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

function f(a: number, b: string): void {
  print('f', a, b);
}

const t: [number, string] = [1, 'x'];

f(...t);
// CHECK: f 1 x

// Spread mixed with ordinary arguments, in any position.
function g(a: number, b: string, c: number): void {
  print('g', a, b, c);
}
g(...t, 2);
// CHECK-NEXT: g 1 x 2
g(3, ...([ 'mid', 4 ]: [string, number]));
// CHECK-NEXT: g 3 mid 4

// An empty tuple contributes no arguments.
const empty: [] = [];
f(...t, ...empty);
// CHECK-NEXT: f 1 x

// The spread source is evaluated exactly once, in place.
function mk(): [number, string] {
  print('mk');
  return [5, 'once'];
}
f(...mk());
// CHECK-NEXT: mk
// CHECK-NEXT: f 5 once

// Tuple elements are absorbed by a rest parameter.
function rest(a: number, ...r: string[]): void {
  print('rest', a, r.length, r[0], r[1]);
}
rest(...([ 6, 'p', 'q' ]: [number, string, string]));
// CHECK-NEXT: rest 6 2 p q

// Methods, constructors and super.
class C {
  x: number;
  y: string;
  constructor(a: number, b: string) {
    this.x = a;
    this.y = b;
  }
  m(a: number, b: string): void {
    print('m', this.x, a, b);
  }
}
const c = new C(...t);
print('new', c.x, c.y);
// CHECK-NEXT: new 1 x
c.m(...t);
// CHECK-NEXT: m 1 1 x

class D extends C {
  constructor(p: [number, string]) {
    super(...p);
  }
}
print('super', new D(t).y);
// CHECK-NEXT: super x

// function.call and $SHBuiltin.call.
function withThis(this: any, a: number, b: string): void {
  print('withThis', a, b);
}
withThis.call(undefined, ...t);
// CHECK-NEXT: withThis 1 x
$SHBuiltin.call(withThis, undefined, ...t);
// CHECK-NEXT: withThis 1 x

// A tuple spread into an untyped callee is also expanded statically; a tuple
// is not iterable, so the dynamic spread path would not work here.
function untyped(a, b) {
  print('untyped', a, b);
}
untyped(...t);
// CHECK-NEXT: untyped 1 x

// Generic type arguments are inferred from the expanded tuple elements.
function id<T>(a: T): T {
  return a;
}
function second<T>(a: number, b: T): T {
  return b;
}
const inferred: string = id(...(['inferred']: [string]));
print(inferred);
// CHECK-NEXT: inferred
print(second(...t));
// CHECK-NEXT: x

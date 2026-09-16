/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -typed -exec %s | %FileCheck --match-full-lines %s

/// Runtime behavior of destructuring parameters on arrow functions: object,
/// tuple, array-with-rest, nested, default values, object-rest, and the
/// contextually-typed (constraint) path.

'use strict';

(function () {

print('arrow-destr-param');
// CHECK-LABEL: arrow-destr-param

// Object destructuring param.
const obj = ({x, y}: {x: number, y: string}): string => y + String(x);
print(obj({x: 1, y: 'a'}));
// CHECK-NEXT: a1

// Tuple destructuring param.
const tup = ([a, b]: [number, number]): number => a + b;
print(tup([3, 4]));
// CHECK-NEXT: 7

// Array destructuring with rest binds a fresh Array<T>.
const arr = ([h, ...t]: Array<number>): number => h + t.length;
print(arr([1, 2, 3, 4]));
// CHECK-NEXT: 4

// Nested destructuring param.
const nested = ({a: {b}}: {a: {b: number}}): number => b;
print(nested({a: {b: 42}}));
// CHECK-NEXT: 42

// Destructuring param with a default value.
const dflt = ({x}: {x: number} = {x: 7}): number => x;
print(dflt());
// CHECK-NEXT: 7
print(dflt({x: 9}));
// CHECK-NEXT: 9

// Object rest in a param.
const orest = ({a, ...rest}: {a: number, b: string, c: boolean}): string =>
  String(a) + rest.b + String(rest.c);
print(orest({a: 1, b: 'x', c: true}));
// CHECK-NEXT: 1xtrue

// Contextually-typed arrow: the expected callback type is the constraint.
function apply(cb: (p: {x: number, y: string}) => string): string {
  return cb({x: 5, y: 'z'});
}
print(apply(({x, y}: {x: number, y: string}): string => y + String(x)));
// CHECK-NEXT: z5

})();

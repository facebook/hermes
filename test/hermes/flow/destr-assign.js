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

/// Runtime behavior of destructuring *assignment* (as opposed to
/// declaration), where the pattern is typed from the right-hand side. A typed
/// source selects the slot-based lowering, so these cover the named-property,
/// rest, indexer and nested forms, plus the untyped fallbacks.

'use strict';

(function () {

print('destr-assign');
// CHECK-LABEL: destr-assign

let n: number = 0;
let s: string = '';

// Named properties read the source's slots directly.
let obj: {p: number, q: string} = {p: 1, q: 'a'};
({p: n, q: s} = obj);
print(n, s);
// CHECK-NEXT: 1 a

// A rest binding allocates a fresh exact object holding the unconsumed
// fields, in source order.
let wide: {p: number, q: string, r: number} = {p: 2, q: 'b', r: 3};
let rest: {q: string, r: number} = {q: '', r: 0};
({p: n, ...rest} = wide);
print(n, rest.q, rest.r);
// CHECK-NEXT: 2 b 3

// Mutating the rest object does not modify the source.
rest.r = 99;
print(wide.r, rest.r);
// CHECK-NEXT: 3 99

// An indexer source resolves every property through the indexer, and the rest
// binding copies the remaining keys.
let idx: {[string]: number} = {a: 4, b: 5, c: 6};
let idxRest: {[string]: number} = {};
({a: n, ...idxRest} = idx);
print(n, idxRest.b, idxRest.c);
// CHECK-NEXT: 4 5 6

// A nested pattern is typed from the field it receives.
let nested: {p: {q: number}} = {p: {q: 7}};
({p: {q: n}} = nested);
print(n);
// CHECK-NEXT: 7

// Tuple and Array<T> sources, including a nested pattern and a rest binding.
let deep: [string, [number]] = ['c', [8]];
[s, [n]] = deep;
print(s, n);
// CHECK-NEXT: c 8

let nums: Array<number> = [9, 10, 11];
let tail: Array<number> = [];
[n, ...tail] = nums;
print(n, tail.length, tail[0], tail[1]);
// CHECK-NEXT: 9 2 10 11

// A hole consumes a tuple element without binding anything.
let pair: [number, string] = [12, 'd'];
[, s] = pair;
print(s);
// CHECK-NEXT: d

// A default is used only when the source property is missing.
let opt: {p: string | void} = {p: undefined};
({p: s = 'fallback'} = opt);
print(s);
// CHECK-NEXT: fallback
opt = {p: 'present'};
({p: s = 'fallback'} = opt);
print(s);
// CHECK-NEXT: present

// A member expression target stores through the object it names.
let target: {s: string} = {s: ''};
({q: target.s} = obj);
print(target.s);
// CHECK-NEXT: a

// An optional source narrows before the pattern is applied.
let maybe: ?{p: number, q: string} = obj;
({p: n} = maybe);
print(n);
// CHECK-NEXT: 1

// An untyped source falls back to the iterator protocol / dynamic loads, and
// each store is narrowed against the target's declared type. A FastArray is
// not iterable, so use a string, which is.
let anyIter: any = 'gh';
let s2: string = '';
[s, s2] = anyIter;
print(s, s2);
// CHECK-NEXT: g h

let anyObj: any = {p: 14, q: 'f'};
({p: n, q: s} = anyObj);
print(n, s);
// CHECK-NEXT: 14 f

// Destructuring assignment evaluates to the right-hand side.
let result: {p: number, q: string} = ({p: n, q: s} = obj);
print(result.p, result.q);
// CHECK-NEXT: 1 a

// An indexer source with a default of a different type: the default is not
// narrowed to the indexer value type.
let idx2: {[string]: number} = {a: 1};
let ns: number | string = 0;
({b: ns = 'x'} = idx2);
print(ns);
// CHECK-NEXT: x
({a: ns = 'x'} = idx2);
print(ns);
// CHECK-NEXT: 1

// The target is evaluated before the property is read and before the
// default.
let box: {s: string} = {s: ''};
let src: {p: string} = {p: 'old'};
function evalTarget(): {s: string} {
  print('target');
  src.p = 'new';
  return box;
}
({p: evalTarget().s} = src);
print(box.s);
// CHECK-NEXT: target
// CHECK-NEXT: new
let opt2: {p: string | void} = {p: undefined};
function dflt(): string {
  print('default');
  return 'd';
}
({p: evalTarget().s = dflt()} = opt2);
// CHECK-NEXT: target
// CHECK-NEXT: default

})();

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test that Object.keys/values/entries return typed arrays.

'use strict';

const o: {[string]: mixed} = {};
o.b = 1;
o.a = 'x';
o['2'] = true;
o['10'] = null;
o['1'] = 5;

/// Index-like keys come first, in ascending order.
print(Object.keys(o).join(','));
// CHECK: 1,2,10,b,a
print(Object.values(o).join(','));
// CHECK-NEXT: 5,true,,1,x

const entries = Object.entries(o);
for (const e of entries) print(e[0], e[1]);
// CHECK-NEXT: 1 5
// CHECK-NEXT: 2 true
// CHECK-NEXT: 10 null
// CHECK-NEXT: b 1
// CHECK-NEXT: a x

/// The key of an entry is typed as a string.
const [k, v] = entries[0];
print(k.length, v);
// CHECK-NEXT: 1 5

/// The elements are typed, so no casts are needed.
const keys: string[] = Object.keys(o);
print(keys[0].length, keys[4]);
// CHECK-NEXT: 1 a

print(Object.keys({}).length, Object.entries({}).length);
// CHECK-NEXT: 0 0

/// Non-enumerable, symbol and inherited properties are excluded.
const base: {[string]: mixed} = {inherited: 1};
const derived: any = globalThis.Object.create(base);
derived.own = 2;
globalThis.Object.defineProperty(derived, 'hidden', {value: 3});
derived[globalThis.Symbol('s')] = 4;
print(Object.keys(derived).join(','));
// CHECK-NEXT: own

/// FastArrays passed through any enumerate their dense indexed elements.
const fast: mixed[] = [3, 'x'];
const fastAny: any = fast;
print(Object.keys(fastAny).join(','));
// CHECK-NEXT: 0,1
print(Object.values(fastAny).join(','));
// CHECK-NEXT: 3,x
for (const e of Object.entries(fastAny)) print(e[0], e[1]);
// CHECK-NEXT: 0 3
// CHECK-NEXT: 1 x

const emptyFast: mixed[] = [];
const emptyFastAny: any = emptyFast;
print(
  Object.keys(emptyFastAny).length,
  Object.values(emptyFastAny).length,
  Object.entries(emptyFastAny).length,
);
// CHECK-NEXT: 0 0 0

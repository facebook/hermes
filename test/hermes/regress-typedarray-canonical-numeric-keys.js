/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s

// A canonical numeric string key (ES2024 7.1.21) on a TypedArray is never an
// ordinary property (ES2024 10.4.5). Writes to such a key that is not a valid
// integer index ("-1", "-0", "1.5", "4294967295", "NaN", ...) are ignored, and
// defining one fails.

'use strict';

print('typedarray-canonical-numeric-keys');
// CHECK-LABEL: typedarray-canonical-numeric-keys

var t = new Int8Array(2);
t[-1] = 1;
t['-0'] = 2;
t[1.5] = 3;
t['1.5'] = 4;
t[4294967295] = 5;
t[NaN] = 6;
t['Infinity'] = 7;
t[-Infinity] = 8;
t.NaN = 9;
t.Infinity = 10;
t[2] = 11;
print(JSON.stringify(Object.getOwnPropertyNames(t)));
// CHECK-NEXT: ["0","1"]
print('-1' in t, t.hasOwnProperty('-0'), t[-1], t['-0'], t.NaN);
// CHECK-NEXT: false false undefined undefined undefined
print(Array.from(t));
// CHECK-NEXT: 0,0

// -0 as a Number converts to the key "0", which is a valid index.
t[-0] = 12;
print(t[0]);
// CHECK-NEXT: 12

// Strings that only look numeric are ordinary properties.
t['01'] = 1;
t['+1'] = 2;
t['1e3'] = 3;
t['-foo'] = 4;
print(JSON.stringify(Object.getOwnPropertyNames(t)));
// CHECK-NEXT: ["0","1","01","+1","1e3","-foo"]

// The write is ignored even when the TypedArray is not extensible.
var frozen = Object.preventExtensions(new Float64Array(1));
frozen[-1] = 1;
print(Object.getOwnPropertyNames(frozen).length);
// CHECK-NEXT: 1

print(Reflect.set(t, '-1', 1), t.hasOwnProperty('-1'));
// CHECK-NEXT: true false

// With a different receiver, nothing is created on the receiver either.
var receiver = {};
print(Reflect.set(t, '-1', 1, receiver), receiver.hasOwnProperty('-1'));
// CHECK-NEXT: true false

// The value is still converted, as in TypedArraySetElement.
var converted = 0;
t[-1] = {
  valueOf() {
    ++converted;
    return 1;
  },
};
print(converted);
// CHECK-NEXT: 1
try {
  new BigInt64Array(1)[-1] = 1;
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError

// Defining such a property fails.
try {
  Object.defineProperty(t, '-1', {value: 1});
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError
print(Reflect.defineProperty(t, '1.5', {value: 1}), t.hasOwnProperty('1.5'));
// CHECK-NEXT: false false
print(
  Reflect.defineProperty(t, 'NaN', {get() {}}),
  Reflect.defineProperty(t, '5', {get() {}}),
);
// CHECK-NEXT: false false
print(JSON.stringify(Object.getOwnPropertyNames(t)));
// CHECK-NEXT: ["0","1","01","+1","1e3","-foo"]

// Reading or testing a numeric key on a TypedArray never consults the
// prototype chain (ES2024 10.4.5.2 [[HasProperty]], 10.4.5.4 [[Get]]).
var keys = ['-1', '-0', '1.5', '5', 'NaN', 'Infinity', 'foo', '01'];
keys.forEach(function (k) {
  Object.defineProperty(Object.prototype, k, {
    get() {
      print('getter called for ' + k);
      return 'proto';
    },
    set(v) {
      print('setter called for ' + k);
    },
    configurable: true,
  });
});

var r = new Int8Array(2);
print(r[-1], r['-0'], r[1.5], r[5], r.NaN, r.Infinity);
// CHECK-NEXT: undefined undefined undefined undefined undefined undefined
print('-1' in r, '-0' in r, 1.5 in r, 5 in r, 'NaN' in r, 'Infinity' in r);
// CHECK-NEXT: false false false false false false
print(Reflect.has(r, '1.5'), Reflect.get(r, 'NaN'));
// CHECK-NEXT: false undefined

// Writing an out-of-range index does not call a setter on the prototype.
r[5] = 1;
print(5 in r);
// CHECK-NEXT: false

// Non-numeric keys still reach the prototype.
print(r.foo, r['01'], 'foo' in r);
// CHECK-NEXT: getter called for foo
// CHECK-NEXT: getter called for 01
// CHECK-NEXT: proto proto true

// A TypedArray in the prototype chain also stops the lookup.
var o = Object.create(r);
print(o[-1], o[5], o.NaN, '-1' in o, 5 in o, o[0]);
// CHECK-NEXT: undefined undefined undefined false false 0

keys.forEach(function (k) {
  delete Object.prototype[k];
});

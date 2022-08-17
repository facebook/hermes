/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

function lshift(x, y) {
  return x << y;
}

print('left shift');
// CHECK-LABEL: left shift

print(this['lshift'](1, 3));
// CHECK-NEXT: 8

print(this['lshift'](1.5, 3));
// CHECK-NEXT: 8

print(this['lshift']('1', 3));
// CHECK-NEXT: 8

print(this['lshift'](0x1, 0x1f));
// CHECK-NEXT: -2147483648

print(this['lshift'](0xdeadbeef, 16));
// CHECK-NEXT: -1091633152

print(this['lshift'](-1, 2));
// CHECK-NEXT: -4

print(this['lshift'](-1, 32));
// CHECK-NEXT: -1

print(this['lshift'](-2147483648, 1));
// CHECK-NEXT: 0


function rshift(x, y) {
  return x >> y;
}

print('right shift');
// CHECK-LABEL: right shift

print(this['rshift'](-14, 2));
// CHECK-NEXT: -4

print(this['rshift'](8, 2));
// CHECK-NEXT: 2

print(this['rshift'](8, 3));
// CHECK-NEXT: 1

print(this['rshift'](18832, 0x82));
// CHECK-NEXT: 4708

print(this['rshift']('9', 1));
// CHECK-NEXT: 4

print(this['rshift'](0xdeadbeef, 16));
// CHECK-NEXT: -8531

function urshift(x, y) {
  return x >>> y;
}

print('unsigned right shift');
// CHECK-LABEL: unsigned right shift

print(this['urshift'](-14, 2));
// CHECK-NEXT: 1073741820

print(this['urshift'](1.5, 2));
// CHECK-NEXT: 0

print(this['urshift']('8', 3));
// CHECK-NEXT: 1

print(this['urshift'](18832, 0x82));
// CHECK-NEXT: 4708

print(this['urshift'](0xdeadbeef, 16));
// CHECK-NEXT: 57005

function band(x, y) {
  return x & y;
}

print('bitwise and');
// CHECK-LABEL: bitwise and

print(this['band'](0xdeadbeef, 0xc0ffee));
// CHECK-NEXT: 8437486

print(this['band']('0xdeadbeef', 0xc0ffee));
// CHECK-NEXT: 8437486

function bor(x, y) {
  return x | y;
}

print('bitwise or');
// CHECK-LABEL: bitwise or

print(this['bor'](0xdeadbeef, 0xc0ffee));
// CHECK-NEXT: -554827793

print(this['bor']('0xdeadbeef', 0xc0ffee));
// CHECK-NEXT: -554827793

function borzero(x) {
  return x | 0;
}

print('bitwise or with zero');
// CHECK-LABEL: bitwise or with zero

print(this['borzero'](0xdeadbeef));
// CHECK-NEXT: -559038737

print(this['borzero']('0xdeadbeef'));
// CHECK-NEXT: -559038737

print(this['borzero'](1.5));
// CHECK-NEXT: 1

print(this['borzero'](-1000.123));
// CHECK-NEXT: -1000

// Numbers larger than a 32-bit integer should be truncated.
print(this['borzero'](0xffffffff + 1));
// CHECK-NEXT: 0

print(this['borzero'](-0xffffffff - 2));
// CHECK-NEXT: -1

print(this['borzero'](1e21));
// CHECK-NEXT: -559939584

function bxor(x, y) {
  return x ^ y;
}

print('bitwise xor');
// CHECK-LABEL: bitwise xor

print(this['bxor'](0xdeadbeef, 0xc0ffee));
// CHECK-NEXT: -563265279

print(this['bxor']('0xdeadbeef', 0xc0ffee));
// CHECK-NEXT: -563265279

function bnot(x) {
  return ~x;
}

print('bitwise not');
// CHECK-LABEL: bitwise not

print(this['bnot'](0xdeadbeef));
// CHECK-NEXT: 559038736

print(this['bnot']('0xdeadbeef'));
// CHECK-NEXT: 559038736

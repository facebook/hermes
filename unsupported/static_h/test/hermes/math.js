/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('Math');
// CHECK-LABEL: Math
print(Math.toString());
// CHECK-NEXT: [object Math]
print(Math.E);
// CHECK-NEXT: 2.718281828459045
print(Math.LN10);
// CHECK-NEXT: 2.302585092994046
print(Math.LN2);
// CHECK-NEXT: 0.6931471805599453
print(Math.LOG2E);
// CHECK-NEXT: 1.4426950408889634
print(Math.LOG10E);
// CHECK-NEXT: 0.4342944819032518
print(Math.PI);
// CHECK-NEXT: 3.141592653589793
print(Math.SQRT1_2);
// CHECK-NEXT: 0.7071067811865476
print(Math.SQRT2);
// CHECK-NEXT: 1.4142135623730951
// Ensure they're not writable
Math.PI = 4;
print(Math.PI);
// CHECK-NEXT: 3.141592653589793

// Note: JS Math functions are deliberately chosen to be compatible with libm
// functions. Therefore we do not need a comprehensive test suite to verify
// the correctness of atan2, etc., because we merely trampoline to the libm
// functions. Sanity tests are sufficient.
// Math.min(), Math.max(), and Math.random() are the exceptions.
var eps = 1E-8;
print(Math.abs(-3) == 3);
// CHECK-NEXT: true
print(Math.acos(1));
// CHECK-NEXT: 0
print(Math.acosh(1));
// CHECK-NEXT: 0
print(Math.asinh(0));
// CHECK-NEXT: 0
print(Math.atanh(1));
// CHECK-NEXT: Infinity
print(Math.abs(2 * Math.asin(1) - Math.PI) <= eps);
// CHECK-NEXT: true
print(Math.abs(2 * Math.atan(Infinity) - Math.PI) <= eps);
// CHECK-NEXT: true
print(Math.abs(Math.atan2(-1 * 0, -3) + Math.PI) <= eps);
// CHECK-NEXT: true
print(Math.cbrt(8));
// CHECK-NEXT: 2
print(Math.ceil(-3.125));
// CHECK-NEXT: -3
print(Math.cos(0));
// CHECK-NEXT: 1
print(Math.abs(Math.exp(1) - Math.E) <= eps);
// CHECK-NEXT: true
print(Math.expm1(0));
// CHECK-NEXT: 0
print(Math.floor(Math.PI));
// CHECK-NEXT: 3
print(Math.log(Math.E * Math.E));
// CHECK-NEXT: 2
print(Math.log(Math.pow(Math.E, 3)));
// CHECK-NEXT: 3
print(Math.log10(1000));
// CHECK-NEXT: 3
print(Math.log2(0.125));
// CHECK-NEXT: -3
print(Math.log1p(0));
// CHECK-NEXT: 0
print(Math.round(Math.SQRT2));
// CHECK-NEXT: 1
print(Math.round(3.5));
// CHECK-NEXT: 4
print(Math.round(-3.5));
// CHECK-NEXT: -3
print(1.0 / Math.round(.5));
// CHECK-NEXT: 1
print(1.0 / Math.round(-.5));
// CHECK-NEXT: -Infinity
print(1.0 / Math.round(.49));
// CHECK-NEXT: Infinity
print(Math.round(Math.pow(2, 52)));
// CHECK-NEXT: 4503599627370496
print(Math.round(4503599627370495.5));
// CHECK-NEXT: 4503599627370496
print(Math.round(4503599627370495));
// CHECK-NEXT: 4503599627370495
print(Math.round(Math.pow(2, 52) + 1));
// CHECK-NEXT: 4503599627370497
print(Math.round(-Math.pow(2, 52) - 1));
// CHECK-NEXT: -4503599627370497
print(1.0 / Math.round(0.49999999999999994));
// CHECK-NEXT: Infinity
print(1.0 / Math.round(-0.49999999999999994));
// CHECK-NEXT: -Infinity
print(Math.sin(-1 * Math.PI * .5));
// CHECK-NEXT: -1
print(1.0 / Math.sin(-0));
// CHECK-NEXT: -Infinity
print(Math.abs(Math.sqrt(2) - Math.SQRT2) <= eps);
// CHECK-NEXT: true
print(Math.abs(Math.tan(Math.PI * .25) - 1) <= eps);
// CHECK-NEXT: true
print(Math.trunc(3.4));
// CHECK-NEXT: 3
print(Math.trunc(-3.4));
// CHECK-NEXT: -3
print(Math.trunc(0.2), 1.0 / Math.trunc(0.2));
// CHECK-NEXT: 0 Infinity
print(Math.trunc(-0.2), 1.0 / Math.trunc(-0.2));
// CHECK-NEXT: 0 -Infinity
print(Math.trunc(0), 1.0 / Math.trunc(0));
// CHECK-NEXT: 0 Infinity
print(Math.trunc(-0), 1.0 / Math.trunc(-0));
// CHECK-NEXT: 0 -Infinity
print(Math.trunc(Infinity));
// CHECK-NEXT: Infinity
print(Math.trunc(-Infinity));
// CHECK-NEXT: -Infinity
print(Math.trunc(NaN));
// CHECK-NEXT: NaN

print('max');
// CHECK-LABEL: max
print(Math.max());
// CHECK-NEXT: -Infinity
print(Math.max(0));
// CHECK-NEXT: 0
print(Math.max(2, "3", -1));
// CHECK-NEXT: 3
print(Math.max(2, 3, -1, NaN, 100));
// CHECK-NEXT: NaN
print(1 / Math.max(-0));
// CHECK-NEXT: -Infinity
print(1 / Math.max(+0, -0, +0, -0));
// CHECK-NEXT: Infinity
print(1 / Math.max(+0, -0, NaN, +0, -0));
// CHECK-NEXT: NaN
var obj1 = {valueOf: function() {print('obj1'); return NaN}};
var obj2 = {valueOf: function() {print('obj2'); return 2}};
print(Math.max(obj1, obj2));
// CHECK-NEXT: obj1
// CHECK-NEXT: obj2
// CHECK-NEXT: NaN
print(Math.max(obj2, obj1));
// CHECK-NEXT: obj2
// CHECK-NEXT: obj1
// CHECK-NEXT: NaN

print('min');
// CHECK-LABEL: min
print(Math.min());
// CHECK-NEXT: Infinity
print(Math.min(0));
// CHECK-NEXT: 0
print(Math.min(2, "3", -1));
// CHECK-NEXT: -1
print(Math.min(2, 3, -1, NaN, 100));
// CHECK-NEXT: NaN
print(1 / Math.min(-0));
// CHECK-NEXT: -Infinity
print(1 / Math.min(+0, -0, +0, -0));
// CHECK-NEXT: -Infinity
print(1 / Math.min(+0, -0, NaN, +0, -0));
// CHECK-NEXT: NaN
var obj1 = {valueOf: function() {print('obj1'); return NaN}};
var obj2 = {valueOf: function() {print('obj2'); return -1000}};
print(Math.min(obj1, obj2));
// CHECK-NEXT: obj1
// CHECK-NEXT: obj2
// CHECK-NEXT: NaN
print(Math.max(obj2, obj1));
// CHECK-NEXT: obj2
// CHECK-NEXT: obj1
// CHECK-NEXT: NaN

var randomVals = [Math.random(), Math.random(), Math.random(),
                  Math.random(), Math.random(), Math.random()];
print(randomVals.every(function(val, idx, arr) {
    return val >= 0 && val < 1 && arr.indexOf(val) === idx;
}));
// CHECK-NEXT: true

print('pow');
// CHECK-LABEL: pow
print(Math.pow(3, NaN));
// CHECK-NEXT: NaN
print(Math.pow(NaN, +0));
// CHECK-NEXT: 1
print(Math.pow(3, -0));
// CHECK-NEXT: 1
print(Math.pow(NaN, 1));
// CHECK-NEXT: NaN
print(Math.pow(2, Infinity));
// CHECK-NEXT: Infinity
print(1.0 / Math.pow(3, -Infinity));
// CHECK-NEXT: Infinity
print(Math.pow(1, Infinity));
// CHECK-NEXT: NaN
print(Math.pow(1, -Infinity));
// CHECK-NEXT: NaN
print(Math.pow(-1, Infinity));
// CHECK-NEXT: NaN
print(Math.pow(-1, -Infinity));
// CHECK-NEXT: NaN
print(1.0 / Math.pow(0.5, Infinity));
// CHECK-NEXT: Infinity
print(Math.pow(0.5, -Infinity));
// CHECK-NEXT: Infinity
print(Math.pow(Infinity, 2));
// CHECK-NEXT: Infinity
print(1.0 / Math.pow(Infinity, -2));
// CHECK-NEXT: Infinity
print(Math.pow(-Infinity, 3));
// CHECK-NEXT: -Infinity
print(Math.pow(-Infinity, 2.4));
// CHECK-NEXT: Infinity
print(1.0 / Math.pow(-Infinity, -5));
// CHECK-NEXT: -Infinity
print(1.0 / Math.pow(-Infinity, -4.4));
// CHECK-NEXT: Infinity
print(1.0 / Math.pow(+0, 3));
// CHECK-NEXT: Infinity
print(Math.pow(+0, -3));
// CHECK-NEXT: Infinity
print(1.0 / Math.pow(-0, 3));
// CHECK-NEXT: -Infinity
print(1.0 / Math.pow(-0, 2.5));
// CHECK-NEXT: Infinity
print(Math.pow(-0, -3));
// CHECK-NEXT: -Infinity
print(Math.pow(-0, -2.5));
// CHECK-NEXT: Infinity
print(Math.pow(-3, 2.4));
// CHECK-NEXT: NaN
print(Math.pow(2, 3));
// CHECK-NEXT: 8
print(Math.pow(4, 0.5));
// CHECK-NEXT: 2

print('hypot');
// CHECK-LABEL: hypot
print(Math.hypot.length);
// CHECK-NEXT: 2
print(Math.hypot(), 1 / Math.hypot());
// CHECK-NEXT: 0 Infinity
print(1 / Math.hypot(0));
// CHECK-NEXT: Infinity
print(1 / Math.hypot(-0));
// CHECK-NEXT: Infinity
print(1 / Math.hypot(0, 0, -0, 0));
// CHECK-NEXT: Infinity
print(Math.hypot(3,4));
// CHECK-NEXT: 5
print(Math.hypot(3,4,5,6,7,8));
// CHECK-NEXT: 14.106735979665885
print(Math.hypot(3, 10e9, 4.6e-20));
// CHECK-NEXT: 10000000000
print(Math.hypot(10e30, 14.2e31, 15.3e31, 17.8e31));
// CHECK-NEXT: 2.745122948066261e+32
print(Math.hypot(Infinity, 3, 4));
// CHECK-NEXT: Infinity
print(Math.hypot(-Infinity, 3, 4));
// CHECK-NEXT: Infinity
print(Math.hypot(-Infinity, NaN));
// CHECK-NEXT: Infinity
print(Math.hypot(3, 4, NaN));
// CHECK-NEXT: NaN

print('fround');
// CHECK-LABEL: fround
print(Math.fround(NaN));
// CHECK-NEXT: NaN
print(1.0 / Math.fround(0));
// CHECK-NEXT: Infinity
print(1.0 / Math.fround(-0));
// CHECK-NEXT: -Infinity
print(Math.fround(Infinity));
// CHECK-NEXT: Infinity
print(Math.fround(-Infinity));
// CHECK-NEXT: -Infinity
print(Math.fround(1.234));
// CHECK-NEXT: 1.2339999675750732
print(Math.fround(1.5));
// CHECK-NEXT: 1.5
print(Math.fround(-4.2));
// CHECK-NEXT: -4.199999809265137
print(Math.fround(-1.79769e+308));
// CHECK-NEXT: -Infinity

print('imul');
// CHECK-LABEL: imul
print(Math.imul(2, 3));
// CHECK-NEXT: 6
print(Math.imul(-2, 3));
// CHECK-NEXT: -6
print(Math.imul(-2, 3));
// CHECK-NEXT: -6
print(Math.imul(0xffffffff, 4));
// CHECK-NEXT: -4
print(Math.imul(0xfffffffd, 4));
// CHECK-NEXT: -12
print(Math.imul(0.2, 5));
// CHECK-NEXT: 0
print(Math.imul(5, 0.2));
// CHECK-NEXT: 0
print(Math.imul(1 << 30, 3));
// CHECK-NEXT: -1073741824
print(Math.imul(1 << 31, 3));
// CHECK-NEXT: -2147483648
print(Math.imul((1 << 31) - 1, 3));
// CHECK-NEXT: 2147483645
print(Math.imul((1 << 31) - 1, (1 << 31) - 1));
// CHECK-NEXT: 1
print(Math.imul(65536, 65536));
// CHECK-NEXT: 0
print(Math.imul(65536, 65536));
// CHECK-NEXT: 0
print(Math.imul(65535, 65536));
// CHECK-NEXT: -65536
print(Math.imul(65536, 65535));
// CHECK-NEXT: -65536
print(Math.imul(65535, 65535));
// CHECK-NEXT: -131071
print(Math.imul(-1.3677434311116809e+25, 0));
// CHECK-NEXT: 0

print('clz32');
// CHECK-LABEL: clz32
print(Math.clz32(1));
// CHECK-NEXT: 31
print(Math.clz32(1000));
// CHECK-NEXT: 22
print(Math.clz32());
// CHECK-NEXT: 32
print(Math.clz32(NaN));
// CHECK-NEXT: 32
print(Math.clz32(Infinity));
// CHECK-NEXT: 32
print(Math.clz32(-Infinity));
// CHECK-NEXT: 32
print(Math.clz32(0));
// CHECK-NEXT: 32
print(Math.clz32(-0));
// CHECK-NEXT: 32
print(Math.clz32(null));
// CHECK-NEXT: 32
print(Math.clz32(undefined));
// CHECK-NEXT: 32
print(Math.clz32('foo'));
// CHECK-NEXT: 32
print(Math.clz32({}));
// CHECK-NEXT: 32
print(Math.clz32([]));
// CHECK-NEXT: 32
print(Math.clz32(3.5));
// CHECK-NEXT: 30
print(Math.clz32(0xffffffff));
// CHECK-NEXT: 0
print(Math.clz32(0x7fffffff));
// CHECK-NEXT: 1

print('sign');
// CHECK-LABEL: sign
print(Math.sign(NaN));
// CHECK-NEXT: NaN
print(Math.sign(+0));
// CHECK-NEXT: 0
print(Math.sign(-0));
// CHECK-NEXT: 0
print(1 / Math.sign(+0));
// CHECK-NEXT: Infinity
print(1 / Math.sign(-0));
// CHECK-NEXT: -Infinity
print(Math.sign(3));
// CHECK-NEXT: 1
print(Math.sign(-3));
// CHECK-NEXT: -1
print(Math.sign(-Infinity));
// CHECK-NEXT: -1
print(Math.sign(Infinity));
// CHECK-NEXT: 1

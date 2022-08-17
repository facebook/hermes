/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print('Number');
// CHECK-LABEL: Number
print(Number())
// CHECK-NEXT: 0
print(Number(1))
// CHECK-NEXT: 1
print(Number(Infinity))
// CHECK-NEXT: Infinity
print(Number("123"))
// CHECK-NEXT: 123
print(Number("123asdf"))
// CHECK-NEXT: NaN
print(Number("0xdead_beef"))
// CHECK-NEXT: NaN
print(Number("1_000"))
// CHECK-NEXT: NaN
var n = new Number(123);
print(n, n.__proto__ === Number.prototype);
// CHECK-NEXT: 123 true

print('constants');
// CHECK-LABEL: constants
print(Number.MAX_VALUE);
// CHECK-NEXT: 1.7976931348623157e+308
print(Number.NaN);
// CHECK-NEXT: NaN
print(Number.MIN_VALUE);
// CHECK-NEXT: 5e-324
print(Number.NEGATIVE_INFINITY);
// CHECK-NEXT: -Infinity
print(Number.POSITIVE_INFINITY);
// CHECK-NEXT: Infinity
print(Number.EPSILON);
// CHECK-NEXT: 2.220446049250313e-16
print(Number.MAX_SAFE_INTEGER === Math.pow(2, 53) - 1);
// CHECK-NEXT: true
print(Number.MIN_SAFE_INTEGER === -(Math.pow(2, 53) - 1));
// CHECK-NEXT: true

print('toString');
// CHECK-LABEL: toString
print(Number.prototype.toString.length);
// CHECK-LABEL: 1
print(Number(1).toString());
// CHECK-NEXT: 1
print(new Number(2).toString());
// CHECK-NEXT: 2
print(new Number(-Infinity).toString());
// CHECK-NEXT: -Infinity
print(new Number(1/-Infinity).toString());
// CHECK-NEXT: 0
print(Number(3).toString(10));
// CHECK-NEXT: 3
print(new Number(4).toString(10));
// CHECK-NEXT: 4
try {Number(5).toString(40);} catch (e) {print("caught:", e.name, e.message);}
// CHECK-NEXT: caught: RangeError {{.*}}
print((NaN).toString(2));
// CHECK-NEXT: NaN
print((Infinity).toString(2));
// CHECK-NEXT: Infinity
print((0).toString(2));
// CHECK-NEXT: 0
print((5).toString(2));
// CHECK-NEXT: 101
print((1e10).toString(2));
// CHECK-NEXT: 1001010100000010111110010000000000
print((0.75).toString(2));
// CHECK-NEXT: 0.11
print((6.75).toString(2));
// CHECK-NEXT: 110.11
print((-6.75).toString(2));
// CHECK-NEXT: -110.11
print((-6.74).toString(2));
// CHECK-NEXT: -110.1011110101110000101000111101011100001010001111011
print((1052728372).toString(36));
// CHECK-NEXT: hermes
print((1052728372.1).toString(36));
// CHECK-NEXT: hermes.3llln
print((630.9079988289651).toString(36));
// CHECK-NEXT: hi.world
print((19239.3230787172).toString(35));
// CHECK-NEXT: foo.bar
print((2147483647.1).toString(2));
// CHECK-NEXT: 1111111111111111111111111111111.000110011001100110011
print((0.5).toString(3));
// CHECK-NEXT: 0.1111111111111111111111111111111112
print((12.124841).toString(16));
// CHECK-NEXT: c.1ff5946c332f
print((-1984.1).toString(36));
// CHECK-NEXT: -1j4.3lllllllc
print((-0.1).toString(36));
// CHECK-NEXT: -0.3lllllllllm
print((0.44).toString(5));
// CHECK-NEXT: 0.210000000000000000000002
print((0.24).toString(5));
// CHECK-NEXT: 0.11
print((0.999999999999999).toString(4));
// CHECK-NEXT: 0.333333333333333333333333232
print(Math.pow(36,60).toString(36));
// CHECK-NEXT: 1000000000000000000000000000000000000000000000000000000000000
var s = (2.2250738585072e-308).toString(36);
print(s.length);
// CHECK-NEXT: 209
print(s.slice(190));
// CHECK-NEXT: 00000000034lmua2oev
print(Array.prototype.every.call(
  s.slice(2, 190),
  function(c) {return c === '0'}
));
// CHECK-NEXT: true

print('toLocaleString');
// CHECK-LABEL: toLocaleString
print((1).toLocaleString());
// CHECK-NEXT: 1

var infinityString = (+Infinity).toLocaleString();
var expected = globalThis.Intl ? "+∞" : "Infinity";
print(infinityString === expected);
// CHECK-NEXT: true

print((-3.14).toLocaleString());
// CHECK-NEXT: -3.14

print('toFixed');
// CHECK-LABEL: toFixed
print((1e21).toFixed());
// CHECK-NEXT: 1e+21
print((1e22).toFixed());
// CHECK-NEXT: 1e+22
print((NaN).toFixed(1));
// CHECK-NEXT: NaN
print((Infinity).toFixed(1));
// CHECK-NEXT: Infinity
print((-Infinity).toFixed(1));
// CHECK-NEXT: -Infinity
print((0).toFixed(4));
// CHECK-NEXT: 0.0000
print((-0.49).toFixed());
// CHECK-NEXT: -0
print((0.49).toFixed());
// CHECK-NEXT: 0
print((0.5).toFixed());
// CHECK-NEXT: 1
print((0.51).toFixed());
// CHECK-NEXT: 1
print((1.5).toFixed());
// CHECK-NEXT: 2
print((2.5).toFixed());
// CHECK-NEXT: 3
print((123.34).toFixed());
// CHECK-NEXT: 123
print((123.34).toFixed(0));
// CHECK-NEXT: 123
print((123.34).toFixed(1));
// CHECK-NEXT: 123.3
print((123.34).toFixed(2));
// CHECK-NEXT: 123.34
print((123.34).toFixed(3));
// CHECK-NEXT: 123.340
print((123.34).toFixed(10));
// CHECK-NEXT: 123.3400000000
print((123.34).toFixed(20));
// CHECK-NEXT: 123.34000000000000341061
print((-123.34).toFixed());
// CHECK-NEXT: -123
print((-123.34).toFixed(4));
// CHECK-NEXT: -123.3400
print((0.5).toFixed(1));
// CHECK-NEXT: 0.5
print((0.54).toFixed(1));
// CHECK-NEXT: 0.5
print((0.55).toFixed(1));
// CHECK-NEXT: 0.6
print((0.56).toFixed(1));
// CHECK-NEXT: 0.6
print((0.55).toFixed(3));
// CHECK-NEXT: 0.550
print((0.125).toFixed(2));
// CHECK-NEXT: 0.13
print((0.135).toFixed(2));
// CHECK-NEXT: 0.14
print((0.0005).toFixed(3));
// CHECK-NEXT: 0.001
print((1234.567).toFixed(20));
// CHECK-NEXT: 1234.56700000000000727596
print((1234.567).toFixed(25));
// CHECK-NEXT: 1234.5670000000000072759576142
print((1000000000000000128).toFixed(0));
// CHECK-NEXT: 1000000000000000128
print((1000000000000000128.19238).toFixed(1));
// CHECK-NEXT: 1000000000000000128.0
try {(123).toFixed(101);} catch (e) {print('caught', e)}
// CHECK-NEXT: caught RangeError: {{.*}}
try {(123).toFixed(-1);} catch (e) {print('caught', e)}
// CHECK-NEXT: caught RangeError: {{.*}}

print('toExponential');
// CHECK-LABEL: toExponential
print((NaN).toExponential(1));
// CHECK-NEXT: NaN
print((Infinity).toExponential(1));
// CHECK-NEXT: Infinity
print((-Infinity).toExponential(1));
// CHECK-NEXT: -Infinity
print((1e100).toExponential());
// CHECK-NEXT: 1e+100
print((1e100).toExponential(2));
// CHECK-NEXT: 1.00e+100
print((1e-100).toExponential());
// CHECK-NEXT: 1e-100
print((0).toExponential());
// CHECK-NEXT: 0e+0
print((0).toExponential(3));
// CHECK-NEXT: 0.000e+0
print((1234.567).toExponential());
// CHECK-NEXT: 1.234567e+3
print((1234.567).toExponential(0));
// CHECK-NEXT: 1e+3
print((1234.567).toExponential(1));
// CHECK-NEXT: 1.2e+3
print((1234.567).toExponential(2));
// CHECK-NEXT: 1.23e+3
print((1234.567).toExponential(3));
// CHECK-NEXT: 1.235e+3
print((1234.567).toExponential(4));
// CHECK-NEXT: 1.2346e+3
print((1234.567).toExponential(5));
// CHECK-NEXT: 1.23457e+3
print((1234.567).toExponential(6));
// CHECK-NEXT: 1.234567e+3
print((1234.567).toExponential(7));
// CHECK-NEXT: 1.2345670e+3
print((1234.567).toExponential(20));
// CHECK-NEXT: 1.23456700000000000728e+3
print((-1234.567).toExponential());
// CHECK-NEXT: -1.234567e+3
print((-1234.567).toExponential(0));
// CHECK-NEXT: -1e+3
print((-1234.567).toExponential(1));
// CHECK-NEXT: -1.2e+3
print((-1234.567).toExponential(2));
// CHECK-NEXT: -1.23e+3
print((-1234.567).toExponential(3));
// CHECK-NEXT: -1.235e+3
print((-1234.567).toExponential(4));
// CHECK-NEXT: -1.2346e+3
print((-1234.567).toExponential(5));
// CHECK-NEXT: -1.23457e+3
print((-1234.567).toExponential(6));
// CHECK-NEXT: -1.234567e+3
print((-1234.567).toExponential(7));
// CHECK-NEXT: -1.2345670e+3
print((-1234.567).toExponential(20));
// CHECK-NEXT: -1.23456700000000000728e+3
print((-1234.567).toExponential(25));
// CHECK-NEXT: -1.2345670000000000072759576e+3
print((.0015).toExponential());
// CHECK-NEXT: 1.5e-3
print((.0015).toExponential(0));
// CHECK-NEXT: 2e-3
print((.0015).toExponential(1));
// CHECK-NEXT: 1.5e-3
print((.0015).toExponential(2));
// CHECK-NEXT: 1.50e-3
print((.0015).toExponential(3));
// CHECK-NEXT: 1.500e-3
print((.0015).toExponential(25));
// CHECK-NEXT: 1.5000000000000000312250226e-3
try {(123).toExponential(101);} catch (e) {print('caught', e)}
// CHECK-NEXT: caught RangeError: {{.*}}
try {(123).toExponential(-1);} catch (e) {print('caught', e)}
// CHECK-NEXT: caught RangeError: {{.*}}

print('toPrecision');
// CHECK-LABEL: toPrecision
print((NaN).toPrecision(100));
// CHECK-NEXT: NaN
print((Infinity).toPrecision(100));
// CHECK-NEXT: Infinity
print((-Infinity).toPrecision(100));
// CHECK-NEXT: -Infinity
print((1e100).toPrecision(4));
// CHECK-NEXT: 1.000e+100
print((-1e100).toPrecision(4));
// CHECK-NEXT: -1.000e+100
print((-1e-100).toPrecision(4));
// CHECK-NEXT: -1.000e-100
print((0).toPrecision(1));
// CHECK-NEXT: 0
print((0).toPrecision(4));
// CHECK-NEXT: 0.000
print((10).toPrecision());
// CHECK-NEXT: 10
print((10).toPrecision(4));
// CHECK-NEXT: 10.00
print((1234.567).toPrecision(1));
// CHECK-NEXT: 1e+3
print((1234.567).toPrecision(2));
// CHECK-NEXT: 1.2e+3
print((1234.567).toPrecision(3));
// CHECK-NEXT: 1.23e+3
print((1234.567).toPrecision(4));
// CHECK-NEXT: 1235
print((1234.567).toPrecision(5));
// CHECK-NEXT: 1234.6
print((1234.567).toPrecision(6));
// CHECK-NEXT: 1234.57
print((1234.567).toPrecision(7));
// CHECK-NEXT: 1234.567
print((1234.567).toPrecision(8));
// CHECK-NEXT: 1234.5670
print((1234.567).toPrecision(21));
// CHECK-NEXT: 1234.56700000000000728
print((-1234.567).toPrecision(1));
// CHECK-NEXT: -1e+3
print((-1234.567).toPrecision(2));
// CHECK-NEXT: -1.2e+3
print((-1234.567).toPrecision(3));
// CHECK-NEXT: -1.23e+3
print((-1234.567).toPrecision(4));
// CHECK-NEXT: -1235
print((-1234.567).toPrecision(5));
// CHECK-NEXT: -1234.6
print((-1234.567).toPrecision(6));
// CHECK-NEXT: -1234.57
print((-1234.567).toPrecision(7));
// CHECK-NEXT: -1234.567
print((-1234.567).toPrecision(8));
// CHECK-NEXT: -1234.5670
print((-1234.567).toPrecision(21));
// CHECK-NEXT: -1234.56700000000000728
print((-1234.567).toPrecision(25));
// CHECK-NEXT: -1234.567000000000007275958
print((.0015).toPrecision(1));
// CHECK-NEXT: 0.002
print((.0015).toPrecision(2));
// CHECK-NEXT: 0.0015
print((-.0015).toPrecision(2));
// CHECK-NEXT: -0.0015
print((-.000000015).toPrecision(2));
// CHECK-NEXT: -1.5e-8
try {print((123).toPrecision(0));} catch (e) {print('caught', e);}
// CHECK-NEXT: caught RangeError: {{.*}}
try {print((123).toPrecision(101));} catch (e) {print('caught', e);}
// CHECK-NEXT: caught RangeError: {{.*}}

print('isFinite');
// CHECK-LABEL: isFinite
print(Number.isFinite('asdf'));
// CHECK-NEXT: false
print(Number.isFinite(NaN));
// CHECK-NEXT: false
print(Number.isFinite(Infinity));
// CHECK-NEXT: false
print(Number.isFinite(3));
// CHECK-NEXT: true

print('isInteger');
// CHECK-LABEL: isInteger
print(Number.isInteger('asdf'));
// CHECK-NEXT: false
print(Number.isInteger(NaN));
// CHECK-NEXT: false
print(Number.isInteger(Infinity));
// CHECK-NEXT: false
print(Number.isInteger(Math.pow(2, 54)));
// CHECK-NEXT: true
print(Number.isInteger(3.2));
// CHECK-NEXT: false
print(Number.isInteger(3));
// CHECK-NEXT: true

print('isNaN');
// CHECK-LABEL: isNaN
print(Number.isNaN('asdf'));
// CHECK-NEXT: false
print(Number.isNaN(NaN));
// CHECK-NEXT: true
print(Number.isNaN(Infinity));
// CHECK-NEXT: false
print(Number.isNaN(3));
// CHECK-NEXT: false

print('isSafeInteger');
// CHECK-LABEL: isSafeInteger
print(Number.isSafeInteger('asdf'));
// CHECK-NEXT: false
print(Number.isSafeInteger(NaN));
// CHECK-NEXT: false
print(Number.isSafeInteger(Infinity));
// CHECK-NEXT: false
print(Number.isSafeInteger(Math.pow(2, 54)));
// CHECK-NEXT: false
print(Number.isSafeInteger(3.2));
// CHECK-NEXT: false
print(Number.isSafeInteger(3));
// CHECK-NEXT: true

print('parsers');
// CHECK-LABEL: parsers
print(Number.parseInt === parseInt);
// CHECK-NEXT: true
print(Number.parseFloat === parseFloat);
// CHECK-NEXT: true

// Make sure that we don't truncate Unicode chars when converting numbers.
print(Number("0x100000000000000000000000000000000000000" + String.fromCharCode(304)));
// CHECK-NEXT: NaN

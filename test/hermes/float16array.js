/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test Float16Array basic functionality
var f16 = new Float16Array(4);
print("Float16Array created");
// CHECK: Float16Array created

print(f16.length);
// CHECK-NEXT: 4

print(Float16Array.BYTES_PER_ELEMENT);
// CHECK-NEXT: 2

print(f16.byteLength);
// CHECK-NEXT: 8

// Test setting and getting values
f16[0] = 1.5;
f16[1] = Math.PI;
f16[2] = 65504;  // max float16 normal value
f16[3] = -Infinity;

print(f16[0]);
// CHECK-NEXT: 1.5

// Math.PI loses precision when converted to float16
print(f16[1]);
// CHECK-NEXT: 3.140625

print(f16[2]);
// CHECK-NEXT: 65504

print(f16[3]);
// CHECK-NEXT: -Infinity

// Test overflow to infinity
f16[0] = 65520;
print(f16[0]);
// CHECK-NEXT: Infinity

// Test NaN
f16[0] = NaN;
print(Number.isNaN(f16[0]));
// CHECK-NEXT: true

// Test Float16Array.from
var f16b = Float16Array.from([1, 2, 3]);
print(f16b.length);
// CHECK-NEXT: 3
print(f16b[0], f16b[1], f16b[2]);
// CHECK-NEXT: 1 2 3

// Test Float16Array.of
var f16c = Float16Array.of(4, 5, 6);
print(f16c.length);
// CHECK-NEXT: 3
print(f16c[0], f16c[1], f16c[2]);
// CHECK-NEXT: 4 5 6

// Test DataView getFloat16/setFloat16
var buf = new ArrayBuffer(4);
var dv = new DataView(buf);
dv.setFloat16(0, 3.14);
print(dv.getFloat16(0));
// CHECK-NEXT: 3.140625

// Test little endian
dv.setFloat16(2, -1.5, true);
print(dv.getFloat16(2, true));
// CHECK-NEXT: -1.5

// Negative zero round-trip
var f16neg = new Float16Array(1);
f16neg[0] = -0;
print(1/f16neg[0] === -Infinity);
// CHECK-NEXT: true

// Smallest positive denormal (2^-24)
f16neg[0] = 5.960464477539063e-08;
print(f16neg[0] === 5.960464477539063e-08);
// CHECK-NEXT: true

// Values that underflow to zero
f16neg[0] = 1e-10;
print(f16neg[0] === 0);
// CHECK-NEXT: true

// Smallest positive normal (2^-14)
f16neg[0] = 0.00006103515625;
print(f16neg[0] === 0.00006103515625);
// CHECK-NEXT: true

// Round-to-nearest-even (tie goes to even mantissa)
// 1.0 has mantissa 0 (even), 1.0 + 2^-11 ties, should round to 1.0
f16neg[0] = 1.00048828125;  // 1 + 2^-11
print(f16neg[0] === 1);
// CHECK-NEXT: true

// 1.0 + 3*2^-11 = 1.00146484375 ties between 1.001953125 and 1.0009765625
// but 1.001953125 has even mantissa (2), so it rounds there
f16neg[0] = 1.00146484375;
print(f16neg[0] === 1.001953125);
// CHECK-NEXT: true

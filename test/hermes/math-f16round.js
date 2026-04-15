/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

print('Math.f16round');
// CHECK-LABEL: Math.f16round

// ToNumber conversion
print(Math.f16round("1.5"));
// CHECK-NEXT: 1.5
print(Math.f16round(undefined));
// CHECK-NEXT: NaN

// NaN passthrough
print(Math.f16round(NaN));
// CHECK-NEXT: NaN

// Signed zeros preserved
print(1 / Math.f16round(0));
// CHECK-NEXT: Infinity
print(1 / Math.f16round(-0));
// CHECK-NEXT: -Infinity

// Infinities preserved
print(Math.f16round(Infinity));
// CHECK-NEXT: Infinity
print(Math.f16round(-Infinity));
// CHECK-NEXT: -Infinity

// Exact float16 values
print(Math.f16round(1.5));
// CHECK-NEXT: 1.5
print(Math.f16round(-1.5));
// CHECK-NEXT: -1.5

// Precision loss
print(Math.f16round(Math.PI));
// CHECK-NEXT: 3.140625

// Max normal float16 (65504), overflow at 65520
print(Math.f16round(65504));
// CHECK-NEXT: 65504
print(Math.f16round(65519));
// CHECK-NEXT: 65504
print(Math.f16round(65520));
// CHECK-NEXT: Infinity

// Smallest normal (2^-14) and smallest denormal (2^-24)
print(Math.f16round(0.00006103515625));
// CHECK-NEXT: 0.00006103515625
print(Math.f16round(5.960464477539063e-8));
// CHECK-NEXT: 5.960464477539063e-8

// Mid-range subnormal: 2^-15
print(Math.f16round(0.000030517578125));
// CHECK-NEXT: 0.000030517578125
// Negative subnormal
print(Math.f16round(-5.960464477539063e-8));
// CHECK-NEXT: -5.960464477539063e-8

// Round-to-nearest-even within subnormal range:
// 1.5 * 2^-24 ties between 2^-24 and 2*2^-24, rounds to even (2*2^-24)
print(Math.f16round(8.940696716308594e-8));
// CHECK-NEXT: 1.1920928955078125e-7
// 2.5 * 2^-24 ties between 2*2^-24 and 3*2^-24, rounds to even (2*2^-24)
print(Math.f16round(1.4901161193847656e-7));
// CHECK-NEXT: 1.1920928955078125e-7

// 2^-25 is exactly halfway between 0 and smallest subnormal, ties to 0 (even)
print(Math.f16round(2.9802322387695312e-8));
// CHECK-NEXT: 0

// Subnormal-to-normal rounding boundary:
// 1023.5 * 2^-24 is halfway between largest subnormal and smallest normal,
// rounds up to 2^-14 (even mantissa)
print(Math.f16round(1023.5 * 5.960464477539063e-8));
// CHECK-NEXT: 0.00006103515625

// Underflow to zero
print(Math.f16round(1e-10));
// CHECK-NEXT: 0

// Round-to-nearest-even: 1.0+2^-11 ties to 1.0 (even mantissa 0)
print(Math.f16round(1.00048828125));
// CHECK-NEXT: 1
// 1.0+3*2^-11 ties to 1.001953125 (even mantissa 2)
print(Math.f16round(1.00146484375));
// CHECK-NEXT: 1.001953125

print(Math.f16round.length);
// CHECK-NEXT: 1

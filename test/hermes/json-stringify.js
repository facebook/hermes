/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print('json.stringify');
//CHECK-LABEL: json.stringify

print(JSON.stringify('𝌆'));
// CHECK-NEXT: "𝌆"
print(JSON.stringify('\uD834\uDF06'));
// CHECK-NEXT: "𝌆"
print(JSON.stringify('\uD834'));
// CHECK-NEXT: "\ud834"
print(JSON.stringify('\uDF06'));
// CHECK-NEXT: "\udf06"
print(JSON.stringify('\uDF06\uD834'));
// CHECK-NEXT: "\udf06\ud834"
print(JSON.stringify('\uDEAD'));
// CHECK-NEXT: "\udead"
print(JSON.stringify('\uD834\uD834\uDF06'));
// CHECK-NEXT: "\ud834𝌆"
print(JSON.stringify('\uD834a'));
// CHECK-NEXT: "\ud834a"
print(JSON.stringify('\uD834\u0400'));
// CHECK-NEXT: "\ud834Ѐ"
print(JSON.stringify("\ud7ff"));
// CHECK-NEXT: "퟿"
print(JSON.stringify("\ud800"));
// CHECK-NEXT: "\ud800"
print(JSON.stringify("\ud937"));
// CHECK-NEXT: "\ud937"
print(JSON.stringify("\uda20"));
// CHECK-NEXT: "\uda20"
print(JSON.stringify("\udbff"));
// CHECK-NEXT: "\udbff"
print(JSON.stringify("\udc00"));
// CHECK-NEXT: "\udc00"
print(JSON.stringify("\udddd"));
// CHECK-NEXT: "\udddd"
print(JSON.stringify("\udeaf"));
// CHECK-NEXT: "\udeaf"
print(JSON.stringify("\udfff"));
// CHECK-NEXT: "\udfff"
print(JSON.stringify("\ue000"));
// CHECK-NEXT: ""
print(JSON.stringify("\ud7ffa"));
// CHECK-NEXT: "퟿a"
print(JSON.stringify("\ud800a"));
// CHECK-NEXT: "\ud800a"
print(JSON.stringify("\ud937a"));
// CHECK-NEXT: "\ud937a"
print(JSON.stringify("\uda20a"));
// CHECK-NEXT: "\uda20a"
print(JSON.stringify("\udbffa"));
// CHECK-NEXT: "\udbffa"
print(JSON.stringify("\udc00a"));
// CHECK-NEXT: "\udc00a"
print(JSON.stringify("\udddda"));
// CHECK-NEXT: "\udddda"
print(JSON.stringify("\udeafa"));
// CHECK-NEXT: "\udeafa"
print(JSON.stringify("\udfffa"));
// CHECK-NEXT: "\udfffa"
print(JSON.stringify("\ue000a"));
// CHECK-NEXT: "a"
print(JSON.stringify("\ud7ff\ud800"));
// CHECK-NEXT: "퟿\ud800"
print(JSON.stringify("\ud800\ud800"));
// CHECK-NEXT: "\ud800\ud800"
print(JSON.stringify("\ud937\ud800"));
// CHECK-NEXT: "\ud937\ud800"
print(JSON.stringify("\uda20\ud800"));
// CHECK-NEXT: "\uda20\ud800"
print(JSON.stringify("\udbff\ud800"));
// CHECK-NEXT: "\udbff\ud800"
print(JSON.stringify("\udc00\ud800"));
// CHECK-NEXT: "\udc00\ud800"
print(JSON.stringify("\udddd\ud800"));
// CHECK-NEXT: "\udddd\ud800"
print(JSON.stringify("\udeaf\ud800"));
// CHECK-NEXT: "\udeaf\ud800"
print(JSON.stringify("\udfff\ud800"));
// CHECK-NEXT: "\udfff\ud800"
print(JSON.stringify("\ue000\ud800"));
// CHECK-NEXT: "\ud800"
print(JSON.stringify("\ud7ff\udc00"));
// CHECK-NEXT: "퟿\udc00"
print(JSON.stringify("\ud800\udc00"));
// CHECK-NEXT: "𐀀"
print(JSON.stringify("\ud937\udc00"));
// CHECK-NEXT: "񝰀"
print(JSON.stringify("\uda20\udc00"));
// CHECK-NEXT: "򘀀"
print(JSON.stringify("\udbff\udc00"));
// CHECK-NEXT: "􏰀"
print(JSON.stringify("\udc00\udc00"));
// CHECK-NEXT: "\udc00\udc00"
print(JSON.stringify("\udddd\udc00"));
// CHECK-NEXT: "\udddd\udc00"
print(JSON.stringify("\udeaf\udc00"));
// CHECK-NEXT: "\udeaf\udc00"
print(JSON.stringify("\udfff\udc00"));
// CHECK-NEXT: "\udfff\udc00"
print(JSON.stringify("\ue000\udc00"));
// CHECK-NEXT: "\udc00"

// Testing number values.
// NaN -> null (JSON spec)
print(JSON.stringify(NaN));
// CHECK-NEXT: null

// +0 and -0 -> 0
print(JSON.stringify(0));
// CHECK-NEXT: 0
print(JSON.stringify(-0));
// CHECK-NEXT: 0

// Infinity -> null
print(JSON.stringify(Infinity));
// CHECK-NEXT: null
print(JSON.stringify(-Infinity));
// CHECK-NEXT: null

// Negative numbers
print(JSON.stringify(-42));
// CHECK-NEXT: -42
print(JSON.stringify(-3.14159));
// CHECK-NEXT: -3.14159
print(JSON.stringify(-0.001));
// CHECK-NEXT: -0.001

// Large integers (n >= k, no decimal point)
print(JSON.stringify(1));
// CHECK-NEXT: 1
print(JSON.stringify(42));
// CHECK-NEXT: 42
print(JSON.stringify(1000));
// CHECK-NEXT: 1000
print(JSON.stringify(12345));
// CHECK-NEXT: 12345
print(JSON.stringify(1000000));
// CHECK-NEXT: 1000000

// Regular decimals (0 < n < k, decimal in middle)
print(JSON.stringify(0.5));
// CHECK-NEXT: 0.5
print(JSON.stringify(1.5));
// CHECK-NEXT: 1.5
print(JSON.stringify(3.14159));
// CHECK-NEXT: 3.14159
print(JSON.stringify(123.456));
// CHECK-NEXT: 123.456

// Small decimals (n <= 0, leading zeros after decimal)
print(JSON.stringify(0.1));
// CHECK-NEXT: 0.1
print(JSON.stringify(0.01));
// CHECK-NEXT: 0.01
print(JSON.stringify(0.001));
// CHECK-NEXT: 0.001
print(JSON.stringify(0.0001));
// CHECK-NEXT: 0.0001
print(JSON.stringify(0.00001));
// CHECK-NEXT: 0.00001
print(JSON.stringify(0.000001));
// CHECK-NEXT: 0.000001

// Scientific notation, k=1 (single digit mantissa)
print(JSON.stringify(1e21));
// CHECK-NEXT: 1e+21
print(JSON.stringify(1e22));
// CHECK-NEXT: 1e+22
print(JSON.stringify(2e25));
// CHECK-NEXT: 2e+25
print(JSON.stringify(1e-7));
// CHECK-NEXT: 1e-7
print(JSON.stringify(1e-10));
// CHECK-NEXT: 1e-10
print(JSON.stringify(3e-15));
// CHECK-NEXT: 3e-15

// Scientific notation, k>1 (multiple digit mantissa)
print(JSON.stringify(1.23e21));
// CHECK-NEXT: 1.23e+21
print(JSON.stringify(9.87654e25));
// CHECK-NEXT: 9.87654e+25
print(JSON.stringify(1.5e30));
// CHECK-NEXT: 1.5e+30
print(JSON.stringify(1.23e-7));
// CHECK-NEXT: 1.23e-7
print(JSON.stringify(9.87e-10));
// CHECK-NEXT: 9.87e-10
print(JSON.stringify(3.14159e-15));
// CHECK-NEXT: 3.14159e-15

// Boundary cases for n in [-5, 21]
// 1e-5 and 1e-6 are still inside [-5, 21] so no scientific notation
print(JSON.stringify(1e-5));
// CHECK-NEXT: 0.00001
print(JSON.stringify(1e-6));
// CHECK-NEXT: 0.000001
// 1e20 is inside [-5, 21] so no scientific notation
print(JSON.stringify(1e20));
// CHECK-NEXT: 100000000000000000000
// 1e21 is outside, so scientific notation
print(JSON.stringify(1e21));
// CHECK-NEXT: 1e+21

// Edge cases with many significant digits
print(JSON.stringify(1.234567890123456e10));
// CHECK-NEXT: 12345678901.23456
print(JSON.stringify(9.999999999999998e15));
// CHECK-NEXT: 9999999999999998
print(JSON.stringify(1.111111111111111e-10));
// CHECK-NEXT: 1.111111111111111e-10

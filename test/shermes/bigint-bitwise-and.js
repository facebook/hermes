/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

function exceptionName(l) {
  try {
    l();
  } catch (e) {
    return e.name;
  }

  return undefined;
}

function typeAndValue(v) {
  // printing hex bigints for more sane comparisons.
  return typeof(v) + " " + v.toString(16);
}

print("BigInt Binary &");
// CHECK-LABEL: BigInt Binary &

print(exceptionName(numberPlusBigInt));
// CHECK-NEXT: TypeError

print(typeAndValue(BigInt(-1) & BigInt(0)));
// CHECK-NEXT: bigint 0

print(typeAndValue(BigInt(-1) & BigInt("0xff")));
// CHECK-NEXT: bigint ff

print(typeAndValue(BigInt(-1) & BigInt("0xffdd")));
// CHECK-NEXT: bigint ffdd

print(typeAndValue(BigInt(-1) & BigInt("0xffaa0020")));
// CHECK-NEXT: bigint ffaa0020

print(typeAndValue(BigInt(-1) & BigInt("0xffff0000aaaa0000")));
// CHECK-NEXT: bigint ffff0000aaaa0000

print(typeAndValue(BigInt(-1) & BigInt("0xfffffffffffffffffffffffffffffffff00000fffffffffff")));
// CHECK-NEXT: bigint fffffffffffffffffffffffffffffffff00000fffffffffff

print(typeAndValue(-BigInt("0x80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000") & -BigInt("0x80")));
// CHECK-NEXT: bigint -80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

// Bitwise AND can no longer be assumed to return number -- it returns
// a numeric when its arguments' types are unknown. This means that
//
//     number + ("thing" & "thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = BinaryOperatorInst '&', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(BigInt(2)&BigInt(1)));
}

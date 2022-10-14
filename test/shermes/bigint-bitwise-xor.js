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

print("BigInt Binary ^");
// CHECK-LABEL: BigInt Binary ^

print(exceptionName(numberPlusBigInt));
// CHECK-NEXT: TypeError

print(typeAndValue(BigInt(-1) ^ BigInt(0)));
// CHECK-NEXT: bigint -1

print(typeAndValue(BigInt(-1) ^ BigInt("0x80808080ffff80808080")));
// CHECK-NEXT: bigint -80808080ffff80808081

print(typeAndValue(BigInt("0xff") ^ BigInt("0x80")));
// CHECK-NEXT: bigint 7f

print(typeAndValue(BigInt("0xffff") ^ BigInt("0x8080")));
// CHECK-NEXT: bigint 7f7f

print(typeAndValue(BigInt("0xffffffff") ^ BigInt("0x80808080")));
// CHECK-NEXT: bigint 7f7f7f7f

print(typeAndValue(BigInt("0xffffffffffffffff") ^ BigInt("0x8080808080808080")));
// CHECK-NEXT: bigint 7f7f7f7f7f7f7f7f

// Bitwise XOR can no longer be assumed to return number -- it returns
// a numeric when its arguments' types are unknown. This means that
//
//     number + ("thing" ^ "thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = BinaryOperatorInst '^', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(BigInt(2)^BigInt(1)));
}

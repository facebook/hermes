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

print("BigInt Binary |");
// CHECK-LABEL: BigInt Binary |

print(exceptionName(numberPlusBigInt));
// CHECK-NEXT: TypeError

print(typeAndValue(BigInt(-1) | BigInt(0)));
// CHECK-NEXT: bigint -1

print(typeAndValue(BigInt(0xff00ff00) | -BigInt("0xffffffffffffffffffffffffffffffffffffffffffffffffff00ff00")));
// CHECK-NEXT: bigint -ffffffffffffffffffffffffffffffffffffffffffffffff00000100

print(typeAndValue(BigInt(0xff00ff00) | -BigInt("0xffffffffffffffffffffffffffffffffffffffffffffffffff00ff00ff")));
// CHECK-NEXT: bigint -ffffffffffffffffffffffffffffffffffffffffffffffffff00ff00ff

print(typeAndValue(BigInt(-1) | BigInt("0x8000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000")));
// CHECK-NEXT: bigint -1

print(typeAndValue(BigInt(1) | -BigInt("0x8000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000")));
// CHECK-NEXT: bigint -7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

print(typeAndValue(BigInt(1) | BigInt(0xf0000)));
// CHECK-NEXT: bigint f0001

print(typeAndValue(BigInt(-0x33) | -BigInt("0x8000000")));
// CHECK-NEXT: bigint -33

// Bitwise OR can no longer be assumed to return number -- it returns
// a numeric when its arguments' types are unknown. This means that
//
//     number + ("thing" | "thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = BinaryOperatorInst '|', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(BigInt(2)|BigInt(1)));
}

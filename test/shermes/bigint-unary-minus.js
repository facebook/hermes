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
  return typeof(v) + " " + v;
}

a=BigInt(1);

print("BigInt Unary Minus");
// CHECK-LABEL: BigInt Unary Minus

print(typeAndValue(a));
// CHECK: bigint 1

print(typeAndValue(-a));
// CHECK: bigint -1

print(exceptionName(numberPlusBigInt));
// CHECK: TypeError

print(typeAndValue(-BigInt("0xff")));
// CHECK: bigint -255

print(typeAndValue(- -BigInt("0xff")));
// CHECK: bigint 255

print(typeAndValue(-BigInt("0xffff")));
// CHECK: bigint -65535

print(typeAndValue(- -BigInt("0xffff")));
// CHECK: bigint 65535

print(typeAndValue(-BigInt("0xffffffff")));
// CHECK: bigint -4294967295

print(typeAndValue(- -BigInt("0xffffffff")));
// CHECK: bigint 4294967295

print(typeAndValue(-BigInt("0xffffffffffffffff")));
// CHECK: bigint -18446744073709551615

print(typeAndValue(- -BigInt("0xffffffffffffffff")));
// CHECK: bigint 18446744073709551615

print(typeAndValue(-BigInt("0x8000000000000000")))
// CHECK: bigint -9223372036854775808

print(typeAndValue(- -BigInt("0x8000000000000000")))
// CHECK: bigint 9223372036854775808

// Unary minus can no longer be assumed to return number -- it returns a
// numeric when its argument's type is unknown. This means that
//
//     number + (-"thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = UnaryOperatorInst '-', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(-a));
}

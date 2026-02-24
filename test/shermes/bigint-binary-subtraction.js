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

print("BigInt Binary -");
// CHECK-LABEL: BigInt Binary -

print(typeAndValue(a));
// CHECK-NEXT: bigint 1

print(typeAndValue(-a));
// CHECK-NEXT: bigint -1

print(exceptionName(numberPlusBigInt));
// CHECK-NEXT: TypeError

print(typeAndValue(BigInt(1024) - BigInt(1024)));
// CHECK-NEXT: bigint 0

print(BigInt(1024) - BigInt(1024) == 0);
// CHECK-NEXT: true

print(typeAndValue(BigInt("0xff") - BigInt(1)));
// CHECK-NEXT: bigint 254

print(typeAndValue(BigInt("0xff") - BigInt(-1)));
// CHECK-NEXT: bigint 256

print(typeAndValue(-BigInt("0xff") - BigInt(1)));
// CHECK-NEXT: bigint -256

print(typeAndValue(-BigInt("0xff") - BigInt(-1)));
// CHECK-NEXT: bigint -254

print(typeAndValue(BigInt("0xffff") - BigInt(1)));
// CHECK-NEXT: bigint 65534

print(typeAndValue(BigInt("0xffff") - BigInt(-1)));
// CHECK-NEXT: bigint 65536

print(typeAndValue(-BigInt("0xffff") - BigInt(1)));
// CHECK-NEXT: bigint -65536

print(typeAndValue(-BigInt("0xffff") - BigInt(-1)));
// CHECK-NEXT: bigint -65534

print(typeAndValue(BigInt("0xffffffff") - BigInt(1)));
// CHECK-NEXT: bigint 4294967294

print(typeAndValue(BigInt("0xffffffff") - BigInt(-1)));
// CHECK-NEXT: bigint 4294967296

print(typeAndValue(-BigInt("0xffffffff") - BigInt(1)));
// CHECK-NEXT: bigint -4294967296

print(typeAndValue(-BigInt("0xffffffff") - BigInt(-1)));
// CHECK-NEXT: bigint -4294967294

print(typeAndValue(BigInt("0xffffffffffffffff") - BigInt(1)));
// CHECK-NEXT: bigint 18446744073709551614

print(typeAndValue(BigInt("0xffffffffffffffff") - BigInt(-1)));
// CHECK-NEXT: bigint 18446744073709551616

print(typeAndValue(-BigInt("0xffffffffffffffff") - BigInt(1)));
// CHECK-NEXT: bigint -18446744073709551616

print(typeAndValue(-BigInt("0xffffffffffffffff") - BigInt(-1)));
// CHECK-NEXT: bigint -18446744073709551614

// Binary subtraction can no longer be assumed to return int -- it returns a
// numeric when its argument's type is unknown. This means that
//
//     number + ("thing" - "thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = BinaryOperatorInst '-', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(a-BigInt(0)));
}

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O %s -dump-bytecode | %FileCheck --check-prefix=CHKBC %s
// RUN: %hermesc -O %s -dump-ir | %FileCheck --check-prefix=CHKIR %s

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

print("BigInt Binary +");
// CHECK-LABEL: BigInt Binary +

print(exceptionName(numberPlusBigInt));
// CHECK-NEXT: TypeError

print(typeAndValue(BigInt(0) + BigInt(0)));
// CHECK-NEXT: bigint 0

print(typeAndValue(BigInt(0) + BigInt(-1)));
// CHECK-NEXT: bigint -1

print(typeAndValue(BigInt(0xff) + BigInt(1)));
// CHECK-NEXT: bigint 256

print(typeAndValue(BigInt(0xff) + BigInt(-1)));
// CHECK-NEXT: bigint 254

print(typeAndValue(-BigInt("0xff") + BigInt(1)));
// CHECK-NEXT: bigint -254

print(typeAndValue(-BigInt("0xff") + BigInt(-1)));
// CHECK-NEXT: bigint -256

print(typeAndValue(BigInt("0xffff") + BigInt(1)));
// CHECK-NEXT: bigint 65536

print(typeAndValue(BigInt("0xffff") + BigInt(-1)));
// CHECK-NEXT: bigint 65534

print(typeAndValue(-BigInt("0xffff") + BigInt(1)));
// CHECK-NEXT: bigint -65534

print(typeAndValue(-BigInt("0xffff") + BigInt(-1)));
// CHECK-NEXT: bigint -65536

print(typeAndValue(BigInt("0xffffffff") + BigInt(1)));
// CHECK-NEXT: bigint 4294967296

print(typeAndValue(BigInt("0xffffffff") + BigInt(-1)));
// CHECK-NEXT: bigint 4294967294

print(typeAndValue(-BigInt("0xffffffff") + BigInt(1)));
// CHECK-NEXT: bigint -4294967294

print(typeAndValue(-BigInt("0xffffffff") + BigInt(-1)));
// CHECK-NEXT: bigint -4294967296

print(typeAndValue(BigInt("0xffffffffffffffff") + BigInt(1)));
// CHECK-NEXT: bigint 18446744073709551616

print(typeAndValue(BigInt("0xffffffffffffffff") + BigInt(-1)));
// CHECK-NEXT: bigint 18446744073709551614

print(typeAndValue(-BigInt("0xffffffffffffffff") + BigInt(1)));
// CHECK-NEXT: bigint -18446744073709551614

print(typeAndValue(-BigInt("0xffffffffffffffff") + BigInt(-1)));
// CHECK-NEXT: bigint -18446744073709551616

// Binary addition can no longer be assumed to return number -- it returns a
// numeric when its argument's type is unknown. This means that
//
//     number + ("thing" + "thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt() {{.*}}
// CHKIR:  %[[N:[0-9]+]] = BinaryOperatorInst '+', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : string|number

function numberPlusBigInt() {
  return (1+(BigInt(2)+BigInt(0)));
}

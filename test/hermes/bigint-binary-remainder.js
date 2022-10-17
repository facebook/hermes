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

print("BigInt Binary %");
// CHECK-LABEL: BigInt Binary %

print(exceptionName(numberPlusBigInt));
// CHECK-NEXT: TypeError

print(exceptionName(() => BigInt(-1) % BigInt(0)));
// CHECK-NEXT: RangeError

print(exceptionName(() => BigInt(0) % BigInt(0)));
// CHECK-NEXT: RangeError

print(typeAndValue(BigInt(0) % BigInt(1)));
// CHECK-NEXT: bigint 0

print(typeAndValue(BigInt(1) % BigInt(2)));
// CHECK-NEXT: bigint 1

print(typeAndValue(BigInt(1) % BigInt(-2)));
// CHECK-NEXT: bigint 1

print(typeAndValue(BigInt(-1) % BigInt(-2)));
// CHECK-NEXT: bigint -1

print(typeAndValue(BigInt(-1) % BigInt(-2)));
// CHECK-NEXT: bigint -1

print(typeAndValue(BigInt("0xff") % BigInt("0x10")));
// CHECK-NEXT: bigint 15

print(typeAndValue(BigInt("0xfffe") % BigInt("0x10")));
// CHECK-NEXT: bigint 14

print(typeAndValue(BigInt("0xfffffffa") % BigInt("0x10")));
// CHECK-NEXT: bigint 10

print(typeAndValue(BigInt("0xfffffffffffffff3") % BigInt("0x10")));
// CHECK-NEXT: bigint 3

print(typeAndValue(BigInt("0xff") % BigInt("0x1000")));
// CHECK-NEXT: bigint 255

print(typeAndValue(BigInt("0xffdd") % BigInt("0x100000")));
// CHECK-NEXT: bigint 65501

print(typeAndValue(BigInt("0xffffffff") % BigInt("0x1000000000")));
// CHECK-NEXT: bigint 4294967295

print(typeAndValue(BigInt("0xffffffffffffffff") % BigInt("0x100000000000000000")));
// CHECK-NEXT: bigint 18446744073709551615

// Division can no longer be assumed to return number -- it returns
// a numeric when its arguments' types are unknown. This means that
//
//     number + ("thing" / "thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt#0#1()#{{[0-9]+}} {{.*}}
// CHKIR:  %[[N:[0-9]+]] = BinaryOperatorInst '%', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(BigInt(2)%BigInt(1)));
}

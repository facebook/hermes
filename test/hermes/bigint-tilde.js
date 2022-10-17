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

a=BigInt(1);

print("BigInt Not");
// CHECK-LABEL: BigInt Not

print(typeAndValue(a));
// CHECK: bigint 1

print(typeAndValue(~a));
// CHECK: bigint -2

print(exceptionName(numberPlusBigInt));
// CHECK: TypeError

print(typeAndValue(~BigInt("0xff")));
// CHECK: bigint -256

print(typeAndValue(~-BigInt("0xff")));
// CHECK: bigint 254

print(typeAndValue(~BigInt("0xffff")));
// CHECK: bigint -65536

print(typeAndValue(~-BigInt("0xffff")));
// CHECK: bigint 65534

print(typeAndValue(~BigInt("0xffffffff")));
// CHECK: bigint -4294967296

print(typeAndValue(~-BigInt("0xffffffff")));
// CHECK: bigint 4294967294

print(typeAndValue(~BigInt("0xffffffffffffffff")));
// CHECK: bigint -18446744073709551616

print(typeAndValue(~-BigInt("0xffffffffffffffff")));
// CHECK: bigint 18446744073709551614

// Tilde (i.e., negation) can no longer be assumed to return int32 -- it returns a
// numeric when its argument's type is unknown. This means that
//
//     number + (~"thing")
//
// can no longer be emitted with an AddN. This is also true for all other
// <foo>N operations.

// CHKBC-LABEL: Function<numberPlusBigInt>({{.*}}):
// CHKBC-NOT:     AddN
// CHKBC:         Add     r{{[0-9]+}},

// CHKIR-LABEL: function numberPlusBigInt#0#1()#{{[0-9]+}} {{.*}}
// CHKIR:  %[[N:[0-9]+]] = UnaryOperatorInst '~', %{{[0-9]+}}
// CHKIR:  %{{[0-9]+}}   = BinaryOperatorInst '+', 1 : number, %[[N]] : number|bigint

function numberPlusBigInt() {
  return (1+(~a));
}

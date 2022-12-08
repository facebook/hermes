/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

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

function numberPlusBigInt() {
  return (1+(-a));
}

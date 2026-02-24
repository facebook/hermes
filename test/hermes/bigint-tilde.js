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

function numberPlusBigInt() {
  return (1+(~a));
}

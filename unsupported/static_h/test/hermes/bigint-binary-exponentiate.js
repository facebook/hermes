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
  // printing hex bigints for more sane comparisons.
  return typeof(v) + " " + v.toString(16);
}

print("BigInt Binary **");
// CHECK-LABEL: BigInt Binary **

print(exceptionName(() => BigInt(1) ** 0));
// CHECK-NEXT: TypeError

print(exceptionName(() => 1 ** BigInt(0)));
// CHECK-NEXT: TypeError

print(exceptionName(() => BigInt(0) ** BigInt(-1)));
// CHECK-NEXT: RangeError

print(exceptionName(() => BigInt(0) ** BigInt(-1024)));
// CHECK-NEXT: RangeError

print(typeAndValue(BigInt(1) ** BigInt(0)));
// CHECK-NEXT: bigint 1

print(typeAndValue(BigInt(0) ** BigInt(0)));
// CHECK-NEXT: bigint 1

print(typeAndValue(BigInt(0) ** BigInt(1024000)));
// CHECK-NEXT: bigint 0

print(typeAndValue(BigInt(2) ** BigInt(10*64)));
// CHECK-NEXT: bigint 10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

print(typeAndValue((-BigInt(2)) ** BigInt(10*64 - 1)));
// CHECK-NEXT: bigint -8000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

print(typeAndValue(BigInt(2) ** BigInt(63)));
// CHECK-NEXT: bigint 8000000000000000

print(typeAndValue((-BigInt(2)) ** BigInt(63)));
// CHECK-NEXT: bigint -8000000000000000

print(typeAndValue(BigInt(2) ** BigInt(64)));
// CHECK-NEXT: bigint 10000000000000000

print(typeAndValue((-BigInt(2)) ** BigInt(63)));
// CHECK-NEXT: bigint -8000000000000000

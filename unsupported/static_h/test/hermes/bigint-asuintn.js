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
  return typeof(v) + " " + v.toString(16);
}

print('BigInt.asUintN');
// CHECK-LABEL: BigInt.asUintN

for (i = 0; i < 71; ++i) {
  print(typeAndValue(BigInt.asUintN(i, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
}
// CHECK-NEXT: bigint 0
// CHECK-NEXT: bigint 1
// CHECK-NEXT: bigint 1
// CHECK-NEXT: bigint 5
// CHECK-NEXT: bigint 5
// CHECK-NEXT: bigint 15
// CHECK-NEXT: bigint 15
// CHECK-NEXT: bigint 55
// CHECK-NEXT: bigint 55
// CHECK-NEXT: bigint 155
// CHECK-NEXT: bigint 155
// CHECK-NEXT: bigint 555
// CHECK-NEXT: bigint 555
// CHECK-NEXT: bigint 1555
// CHECK-NEXT: bigint 1555
// CHECK-NEXT: bigint 5555
// CHECK-NEXT: bigint 5555
// CHECK-NEXT: bigint 15555
// CHECK-NEXT: bigint 15555
// CHECK-NEXT: bigint 55555
// CHECK-NEXT: bigint 55555
// CHECK-NEXT: bigint 155555
// CHECK-NEXT: bigint 155555
// CHECK-NEXT: bigint 555555
// CHECK-NEXT: bigint 555555
// CHECK-NEXT: bigint 1555555
// CHECK-NEXT: bigint 1555555
// CHECK-NEXT: bigint 5555555
// CHECK-NEXT: bigint 5555555
// CHECK-NEXT: bigint 15555555
// CHECK-NEXT: bigint 15555555
// CHECK-NEXT: bigint 55555555
// CHECK-NEXT: bigint 55555555
// CHECK-NEXT: bigint 155555555
// CHECK-NEXT: bigint 155555555
// CHECK-NEXT: bigint 555555555
// CHECK-NEXT: bigint 555555555
// CHECK-NEXT: bigint 1555555555
// CHECK-NEXT: bigint 1555555555
// CHECK-NEXT: bigint 5555555555
// CHECK-NEXT: bigint 5555555555
// CHECK-NEXT: bigint 15555555555
// CHECK-NEXT: bigint 15555555555
// CHECK-NEXT: bigint 55555555555
// CHECK-NEXT: bigint 55555555555
// CHECK-NEXT: bigint 155555555555
// CHECK-NEXT: bigint 155555555555
// CHECK-NEXT: bigint 555555555555
// CHECK-NEXT: bigint 555555555555
// CHECK-NEXT: bigint 1555555555555
// CHECK-NEXT: bigint 1555555555555
// CHECK-NEXT: bigint 5555555555555
// CHECK-NEXT: bigint 5555555555555
// CHECK-NEXT: bigint 15555555555555
// CHECK-NEXT: bigint 15555555555555
// CHECK-NEXT: bigint 55555555555555
// CHECK-NEXT: bigint 55555555555555
// CHECK-NEXT: bigint 155555555555555
// CHECK-NEXT: bigint 155555555555555
// CHECK-NEXT: bigint 555555555555555
// CHECK-NEXT: bigint 555555555555555
// CHECK-NEXT: bigint 1555555555555555
// CHECK-NEXT: bigint 1555555555555555
// CHECK-NEXT: bigint 5555555555555555
// CHECK-NEXT: bigint 5555555555555555
// CHECK-NEXT: bigint 15555555555555555
// CHECK-NEXT: bigint 15555555555555555
// CHECK-NEXT: bigint 55555555555555555
// CHECK-NEXT: bigint 55555555555555555
// CHECK-NEXT: bigint 155555555555555555
// CHECK-NEXT: bigint 155555555555555555

print(typeAndValue(BigInt.asUintN(259, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 55555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asUintN(260, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 55555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asUintN(261, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asUintN(262, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asUintN(280, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asUintN(300, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asUintN(5000, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

for (i = 0; i < 71; ++i) {
  print(typeAndValue(BigInt.asUintN(i, BigInt(-1))));
}
// CHECK-NEXT: bigint 0
// CHECK-NEXT: bigint 1
// CHECK-NEXT: bigint 3
// CHECK-NEXT: bigint 7
// CHECK-NEXT: bigint f
// CHECK-NEXT: bigint 1f
// CHECK-NEXT: bigint 3f
// CHECK-NEXT: bigint 7f
// CHECK-NEXT: bigint ff
// CHECK-NEXT: bigint 1ff
// CHECK-NEXT: bigint 3ff
// CHECK-NEXT: bigint 7ff
// CHECK-NEXT: bigint fff
// CHECK-NEXT: bigint 1fff
// CHECK-NEXT: bigint 3fff
// CHECK-NEXT: bigint 7fff
// CHECK-NEXT: bigint ffff
// CHECK-NEXT: bigint 1ffff
// CHECK-NEXT: bigint 3ffff
// CHECK-NEXT: bigint 7ffff
// CHECK-NEXT: bigint fffff
// CHECK-NEXT: bigint 1fffff
// CHECK-NEXT: bigint 3fffff
// CHECK-NEXT: bigint 7fffff
// CHECK-NEXT: bigint ffffff
// CHECK-NEXT: bigint 1ffffff
// CHECK-NEXT: bigint 3ffffff
// CHECK-NEXT: bigint 7ffffff
// CHECK-NEXT: bigint fffffff
// CHECK-NEXT: bigint 1fffffff
// CHECK-NEXT: bigint 3fffffff
// CHECK-NEXT: bigint 7fffffff
// CHECK-NEXT: bigint ffffffff
// CHECK-NEXT: bigint 1ffffffff
// CHECK-NEXT: bigint 3ffffffff
// CHECK-NEXT: bigint 7ffffffff
// CHECK-NEXT: bigint fffffffff
// CHECK-NEXT: bigint 1fffffffff
// CHECK-NEXT: bigint 3fffffffff
// CHECK-NEXT: bigint 7fffffffff
// CHECK-NEXT: bigint ffffffffff
// CHECK-NEXT: bigint 1ffffffffff
// CHECK-NEXT: bigint 3ffffffffff
// CHECK-NEXT: bigint 7ffffffffff
// CHECK-NEXT: bigint fffffffffff
// CHECK-NEXT: bigint 1fffffffffff
// CHECK-NEXT: bigint 3fffffffffff
// CHECK-NEXT: bigint 7fffffffffff
// CHECK-NEXT: bigint ffffffffffff
// CHECK-NEXT: bigint 1ffffffffffff
// CHECK-NEXT: bigint 3ffffffffffff
// CHECK-NEXT: bigint 7ffffffffffff
// CHECK-NEXT: bigint fffffffffffff
// CHECK-NEXT: bigint 1fffffffffffff
// CHECK-NEXT: bigint 3fffffffffffff
// CHECK-NEXT: bigint 7fffffffffffff
// CHECK-NEXT: bigint ffffffffffffff
// CHECK-NEXT: bigint 1ffffffffffffff
// CHECK-NEXT: bigint 3ffffffffffffff
// CHECK-NEXT: bigint 7ffffffffffffff
// CHECK-NEXT: bigint fffffffffffffff
// CHECK-NEXT: bigint 1fffffffffffffff
// CHECK-NEXT: bigint 3fffffffffffffff
// CHECK-NEXT: bigint 7fffffffffffffff
// CHECK-NEXT: bigint ffffffffffffffff
// CHECK-NEXT: bigint 1ffffffffffffffff
// CHECK-NEXT: bigint 3ffffffffffffffff
// CHECK-NEXT: bigint 7ffffffffffffffff
// CHECK-NEXT: bigint fffffffffffffffff
// CHECK-NEXT: bigint 1fffffffffffffffff
// CHECK-NEXT: bigint 3fffffffffffffffff

print(typeAndValue(BigInt.asUintN(5000, BigInt(-1))));
// CHECK-NEXT: bigint ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

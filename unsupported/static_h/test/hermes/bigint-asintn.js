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

print('BigInt.asIntN');
// CHECK-LABEL: BigInt.asIntN

for (i = 0; i < 71; ++i) {
  print(typeAndValue(BigInt.asIntN(i, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
}
// CHECK-NEXT: bigint 0
// CHECK-NEXT: bigint -1
// CHECK-NEXT: bigint 1
// CHECK-NEXT: bigint -3
// CHECK-NEXT: bigint 5
// CHECK-NEXT: bigint -b
// CHECK-NEXT: bigint 15
// CHECK-NEXT: bigint -2b
// CHECK-NEXT: bigint 55
// CHECK-NEXT: bigint -ab
// CHECK-NEXT: bigint 155
// CHECK-NEXT: bigint -2ab
// CHECK-NEXT: bigint 555
// CHECK-NEXT: bigint -aab
// CHECK-NEXT: bigint 1555
// CHECK-NEXT: bigint -2aab
// CHECK-NEXT: bigint 5555
// CHECK-NEXT: bigint -aaab
// CHECK-NEXT: bigint 15555
// CHECK-NEXT: bigint -2aaab
// CHECK-NEXT: bigint 55555
// CHECK-NEXT: bigint -aaaab
// CHECK-NEXT: bigint 155555
// CHECK-NEXT: bigint -2aaaab
// CHECK-NEXT: bigint 555555
// CHECK-NEXT: bigint -aaaaab
// CHECK-NEXT: bigint 1555555
// CHECK-NEXT: bigint -2aaaaab
// CHECK-NEXT: bigint 5555555
// CHECK-NEXT: bigint -aaaaaab
// CHECK-NEXT: bigint 15555555
// CHECK-NEXT: bigint -2aaaaaab
// CHECK-NEXT: bigint 55555555
// CHECK-NEXT: bigint -aaaaaaab
// CHECK-NEXT: bigint 155555555
// CHECK-NEXT: bigint -2aaaaaaab
// CHECK-NEXT: bigint 555555555
// CHECK-NEXT: bigint -aaaaaaaab
// CHECK-NEXT: bigint 1555555555
// CHECK-NEXT: bigint -2aaaaaaaab
// CHECK-NEXT: bigint 5555555555
// CHECK-NEXT: bigint -aaaaaaaaab
// CHECK-NEXT: bigint 15555555555
// CHECK-NEXT: bigint -2aaaaaaaaab
// CHECK-NEXT: bigint 55555555555
// CHECK-NEXT: bigint -aaaaaaaaaab
// CHECK-NEXT: bigint 155555555555
// CHECK-NEXT: bigint -2aaaaaaaaaab
// CHECK-NEXT: bigint 555555555555
// CHECK-NEXT: bigint -aaaaaaaaaaab
// CHECK-NEXT: bigint 1555555555555
// CHECK-NEXT: bigint -2aaaaaaaaaaab
// CHECK-NEXT: bigint 5555555555555
// CHECK-NEXT: bigint -aaaaaaaaaaaab
// CHECK-NEXT: bigint 15555555555555
// CHECK-NEXT: bigint -2aaaaaaaaaaaab
// CHECK-NEXT: bigint 55555555555555
// CHECK-NEXT: bigint -aaaaaaaaaaaaab
// CHECK-NEXT: bigint 155555555555555
// CHECK-NEXT: bigint -2aaaaaaaaaaaaab
// CHECK-NEXT: bigint 555555555555555
// CHECK-NEXT: bigint -aaaaaaaaaaaaaab
// CHECK-NEXT: bigint 1555555555555555
// CHECK-NEXT: bigint -2aaaaaaaaaaaaaab
// CHECK-NEXT: bigint 5555555555555555
// CHECK-NEXT: bigint -aaaaaaaaaaaaaaab
// CHECK-NEXT: bigint 15555555555555555
// CHECK-NEXT: bigint -2aaaaaaaaaaaaaaab
// CHECK-NEXT: bigint 55555555555555555
// CHECK-NEXT: bigint -aaaaaaaaaaaaaaaab
// CHECK-NEXT: bigint 155555555555555555

print(typeAndValue(BigInt.asIntN(259, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint -2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab

print(typeAndValue(BigInt.asIntN(260, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 55555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(261, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint -aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab

print(typeAndValue(BigInt.asIntN(262, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(280, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(300, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(5000, BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 155555555555555555555555555555555555555555555555555555555555555555


for (i = 0; i < 71; ++i) {
  print(typeAndValue(BigInt.asIntN(i, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
}
// CHECK-NEXT: bigint 0
// CHECK-NEXT: bigint -1
// CHECK-NEXT: bigint -1
// CHECK-NEXT: bigint 3
// CHECK-NEXT: bigint -5
// CHECK-NEXT: bigint b
// CHECK-NEXT: bigint -15
// CHECK-NEXT: bigint 2b
// CHECK-NEXT: bigint -55
// CHECK-NEXT: bigint ab
// CHECK-NEXT: bigint -155
// CHECK-NEXT: bigint 2ab
// CHECK-NEXT: bigint -555
// CHECK-NEXT: bigint aab
// CHECK-NEXT: bigint -1555
// CHECK-NEXT: bigint 2aab
// CHECK-NEXT: bigint -5555
// CHECK-NEXT: bigint aaab
// CHECK-NEXT: bigint -15555
// CHECK-NEXT: bigint 2aaab
// CHECK-NEXT: bigint -55555
// CHECK-NEXT: bigint aaaab
// CHECK-NEXT: bigint -155555
// CHECK-NEXT: bigint 2aaaab
// CHECK-NEXT: bigint -555555
// CHECK-NEXT: bigint aaaaab
// CHECK-NEXT: bigint -1555555
// CHECK-NEXT: bigint 2aaaaab
// CHECK-NEXT: bigint -5555555
// CHECK-NEXT: bigint aaaaaab
// CHECK-NEXT: bigint -15555555
// CHECK-NEXT: bigint 2aaaaaab
// CHECK-NEXT: bigint -55555555
// CHECK-NEXT: bigint aaaaaaab
// CHECK-NEXT: bigint -155555555
// CHECK-NEXT: bigint 2aaaaaaab
// CHECK-NEXT: bigint -555555555
// CHECK-NEXT: bigint aaaaaaaab
// CHECK-NEXT: bigint -1555555555
// CHECK-NEXT: bigint 2aaaaaaaab
// CHECK-NEXT: bigint -5555555555
// CHECK-NEXT: bigint aaaaaaaaab
// CHECK-NEXT: bigint -15555555555
// CHECK-NEXT: bigint 2aaaaaaaaab
// CHECK-NEXT: bigint -55555555555
// CHECK-NEXT: bigint aaaaaaaaaab
// CHECK-NEXT: bigint -155555555555
// CHECK-NEXT: bigint 2aaaaaaaaaab
// CHECK-NEXT: bigint -555555555555
// CHECK-NEXT: bigint aaaaaaaaaaab
// CHECK-NEXT: bigint -1555555555555
// CHECK-NEXT: bigint 2aaaaaaaaaaab
// CHECK-NEXT: bigint -5555555555555
// CHECK-NEXT: bigint aaaaaaaaaaaab
// CHECK-NEXT: bigint -15555555555555
// CHECK-NEXT: bigint 2aaaaaaaaaaaab
// CHECK-NEXT: bigint -55555555555555
// CHECK-NEXT: bigint aaaaaaaaaaaaab
// CHECK-NEXT: bigint -155555555555555
// CHECK-NEXT: bigint 2aaaaaaaaaaaaab
// CHECK-NEXT: bigint -555555555555555
// CHECK-NEXT: bigint aaaaaaaaaaaaaab
// CHECK-NEXT: bigint -1555555555555555
// CHECK-NEXT: bigint 2aaaaaaaaaaaaaab
// CHECK-NEXT: bigint -5555555555555555
// CHECK-NEXT: bigint aaaaaaaaaaaaaaab
// CHECK-NEXT: bigint -15555555555555555
// CHECK-NEXT: bigint 2aaaaaaaaaaaaaaab
// CHECK-NEXT: bigint -55555555555555555
// CHECK-NEXT: bigint aaaaaaaaaaaaaaaab
// CHECK-NEXT: bigint -155555555555555555

print(typeAndValue(BigInt.asIntN(259, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint 2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab

print(typeAndValue(BigInt.asIntN(260, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint -55555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(261, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab

print(typeAndValue(BigInt.asIntN(262, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint -155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(280, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint -155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(300, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint -155555555555555555555555555555555555555555555555555555555555555555

print(typeAndValue(BigInt.asIntN(5000, -BigInt("0x155555555555555555555555555555555555555555555555555555555555555555"))));
// CHECK-NEXT: bigint -155555555555555555555555555555555555555555555555555555555555555555

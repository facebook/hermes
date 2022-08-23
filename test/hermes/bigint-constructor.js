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

print('BigInt');
// CHECK-LABEL: BigInt

print(BigInt.prototype.constructor == BigInt);
// CHECK-NEXT: true

print(exceptionName(() => BigInt()));
// CHECK-NEXT: TypeError

print(exceptionName(() => new BigInt()));
// CHECK-NEXT: TypeError

print(typeAndValue(BigInt(1)));
// CHECK-NEXT: bigint 1

print(typeAndValue(BigInt('1')));
// CHECK-NEXT: bigint 1

print(typeAndValue(BigInt(new Boolean(true))));
// CHECK-NEXT: bigint 1

print(typeAndValue(BigInt('-1234567890123456789012345678901234567890123456789012345678901234567890123456789')));
// CHECK-NEXT: bigint -1234567890123456789012345678901234567890123456789012345678901234567890123456789

print(exceptionName(() => BigInt("invalid bigint literal")));
// CHECK-NEXT: SyntaxError

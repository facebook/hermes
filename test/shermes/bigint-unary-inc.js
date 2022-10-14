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

print("BigInt Increment (++)");
// CHECK-LABEL: BigInt Increment (++)

a = BigInt(1);

print(typeAndValue(a++));
// CHECK: bigint 1

print(typeAndValue(++a));
// CHECK: bigint 3

print(exceptionName(() => print("compiler is optimizing ToNumber after Inc", +(++a))));
// CHECK: TypeError

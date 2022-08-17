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

print("BigInt Binary >>>");
// CHECK-LABEL: BigInt Binary >>>

print(exceptionName(() => BigInt(-1) >>> BigInt(10)));
// CHECK-NEXT: TypeError

print(exceptionName(() => BigInt(-1) >>> 10));
// CHECK-NEXT: TypeError

print(exceptionName(() => -1 >>> BigInt(10)));
// CHECK-NEXT: TypeError

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

print('BigInt.prototype.valueOf');
// CHECK-LABEL: BigInt.prototype.valueOf

var n = BigInt(1)

print(BigInt.prototype.valueOf === n.valueOf);
// CHECK: true

print(typeAndValue(n.valueOf()));
// CHECK: bigint 1

var x = { valueOf: n.valueOf };

print(exceptionName(() => x.valueOf()));
// CHECK: TypeError

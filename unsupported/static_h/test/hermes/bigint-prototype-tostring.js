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

print('BigInt.prototype.toString');
// CHECK-LABEL: BigInt.prototype.toString

var n = BigInt(1)

print(BigInt.prototype.toString === n.toString);
// CHECK: true

print(n.toString());
// CHECK: 1

var m = BigInt("-123456789123456789")

print(m.toString());
// CHECK: -123456789123456789

print(m.toString(32));
// CHECK: -3dkr9emd0nol

print(m.toString("22"));
// CHECK: -9d6544j1ahgbb

print(m.toString(undefined));
// CHECK: -123456789123456789

print(exceptionName(() => m.toString(BigInt(32))));
// CHECK: TypeError

print(exceptionName(() => m.toString(37)));
// CHECK: RangeError

print(exceptionName(() => m.toString("22n")));
// CHECK: RangeError

print(exceptionName(() => m.toString(null)));
// CHECK: RangeError

print(exceptionName(() => m.toString({})));
// CHECK: RangeError

var x = { toString: n.toString };

print(exceptionName(() => x.toString()));
// CHECK: TypeError

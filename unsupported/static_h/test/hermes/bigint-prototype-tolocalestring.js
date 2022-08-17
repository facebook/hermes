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

print('BigInt.prototype.toLocaleString');
// CHECK-LABEL: BigInt.prototype.toLocaleString

var n = BigInt("123456789")

print(BigInt.prototype.toLocaleString === n.toLocaleString);
// CHECK: true

print(n.toLocaleString());
// CHECK: 123456789

var m = BigInt("-123456789")

print(m.toLocaleString());
// CHECK: -123456789

var x = { toLocaleString: n.toLocaleString };

print(exceptionName(() => x.toLocaleString()));
// CHECK: TypeError

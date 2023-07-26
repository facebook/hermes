/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var str = 'AAAAAAAAAAAAAAAA';
for (var i = 0; i < 24; i++) {
    str += str;
}
var obj = {};
// str is a very long string
obj[Symbol.toStringTag] = str;
try {
  var result = 2 * obj;
} catch (e) {
  print(e.name, e.message);
}
// CHECK: RangeError String length exceeds limit

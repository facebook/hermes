/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ulimit -s 1024 && %hermes -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: ulimit -s 1024 && %shermes -exec -Wx,-gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s

var a = [1];
for (var i = 0; i < 100000; ++i) {
  a = [a];
}
try { a.flat(Infinity); } catch(e) { print('caught', e.name) }
// CHECK: caught RangeError

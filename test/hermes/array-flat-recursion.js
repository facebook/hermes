/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//RUN: ulimit -s 512 && %hermes -O %s -gc-sanitize-handles=0 | %FileCheck --match-full-lines %s

var a = [1];
for (var i = 0; i < 1000; ++i) {
  a = [a];
}
try { a.flat(Infinity); } catch(e) { print('caught', e.name) }
// CHECK: caught RangeError

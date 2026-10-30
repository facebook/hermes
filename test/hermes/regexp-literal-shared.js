/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s

// Regexps from one literal share bytecode but nothing else: lastIndex,
// match progress and named groups stay per object.

function main() {
  function mk() { return /a(?<n>\d)/g; }
  var a = mk(), b = mk();
  print(a === b);
// CHECK: false
  print(a.exec("a1a2")[0], a.lastIndex, b.lastIndex);
// CHECK-NEXT: a1 2 0
  print(b.exec("xa3")[0], b.lastIndex, a.lastIndex);
// CHECK-NEXT: a3 3 2
  print(a.exec("a1a2").groups.n, a.lastIndex);
// CHECK-NEXT: 2 4
  print(mk().test("a5"), mk().lastIndex);
// CHECK-NEXT: true 0
  var s = /b/y;
  print("abc".split(s).length, s.sticky, s.lastIndex);
// CHECK-NEXT: 2 true 0
  // split() cleared sticky on its private copy only.
  print(s.exec("ab") === null);
// CHECK-NEXT: true
}
main();

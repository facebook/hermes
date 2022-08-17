// Remove this once the REPL can handle block comments.
// @lint-ignore-every LICENSELINT

// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// RUN: cat %s | %hermes -prompt="" -prompt2="" | %FileCheck --match-full-lines %s

"multiline"
// CHECK-LABEL: "multiline"
x = {foo: function() {
  return 42;
}}
// CHECK-NEXT: { foo: [Function foo] }
a = [ 1,2,3,
  4,5,6,
  7,8,9]
// CHECK-NEXT: [ 1, 2, 3, 4, 5, 6, 7, 8, 9, [length]: 9 ]
"two\
lines"
// CHECK-NEXT: "twolines"

// Remove this once the REPL can handle block comments.
// @lint-ignore-every LICENSELINT

// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// RUN: cat %s | %hermes -prompt="" -prompt2="" | %FileCheck --match-full-lines %s

foo = 2;

// The following statements use `/` as division.
(1 / 2);
// CHECK: 0.5
x = (1 / (foo) / 1);
// CHECK: 0.5
(
  1 / 2
);
// CHECK: 0.5

// The following statements use `/` for a regex.
(/foo()/);
// CHECK: /foo()/
r = /foo\(/;
// CHECK: /foo\(/
((/foo\(/));
// CHECK: /foo\(/
(
  /foo\)/
)
// CHECK: /foo\)/

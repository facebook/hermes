// Remove this once the REPL can handle block comments.
// @lint-ignore-every LICENSELINT

// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// RUN: cat %s | %hermes -prompt="" -prompt2="" | %FileCheck --match-full-lines %s

new Set[1,2,3])
// CHECK: Uncaught SyntaxError: 1:15:';' expected
// CHECK-NEXT: at eval (native)
// CHECK-NEXT: at evaluateLine ({{.*}})

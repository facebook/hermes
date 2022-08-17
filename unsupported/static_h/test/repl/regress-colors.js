// Remove this once the REPL can handle block comments.
// @lint-ignore-every LICENSELINT

// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// RUN: cat %s | %hermes -prompt="" -prompt2="" | %FileCheck --match-full-lines %s

throw new SyntaxError();
// CHECK: Uncaught SyntaxError
// CHECK-NEXT: at global ({{.*}})
// CHECK-NEXT: at eval (native)
// CHECK-NEXT: at evaluateLine {{.*}}

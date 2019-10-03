// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: cat %s | %repl -prompt "" -prompt2 "" | %FileCheck --match-full-lines %s

new Set[1,2,3])
// CHECK: SyntaxError: 1:15:';' expected
// CHECK-NEXT: at eval (native)
// CHECK-NEXT: at evaluateLine ({{.*}})

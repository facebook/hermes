// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: cat %s | %repl -prompt "" -prompt2 "" | %FileCheck --match-full-lines %s

throw new SyntaxError();
// CHECK: SyntaxError
// CHECK-NEXT: at global ({{.*}})
// CHECK-NEXT: at eval (native)
// CHECK-NEXT: at evaluateLine {{.*}}

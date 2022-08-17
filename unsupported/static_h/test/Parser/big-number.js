/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheck %s --match-full-lines

// This test ensures that a very large numeric literal can be parsed as
// Infinity.
// It is only testing the parser, so it should use dump-ast; however, Infinity
// is not serializable in JSON, so this test needs to happen at the IR level.

55e55555555555555555555555555555555555;

// CHECK-LABEL: function global()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:   %1 = StoreStackInst undefined : undefined, %0
// CHECK-NEXT:   %2 = StoreStackInst Infinity : number, %0


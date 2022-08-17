/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function foo () {
    debugger;
}
//CHECK-LABEL: function foo()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = DebuggerInst
//CHECK-NEXT:     %1 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

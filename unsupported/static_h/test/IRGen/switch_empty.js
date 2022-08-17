/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function empty () {
    switch (1) {}
}
//CHECK-LABEL: function empty()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = BranchInst %BB1
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %1 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

function onlyDefault () {
    switch (1) {
        default: break;
    }
}
//CHECK-LABEL: function onlyDefault()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = BranchInst %BB1
//CHECK-NEXT:   %BB2:
//CHECK-NEXT:     %1 = ReturnInst undefined : undefined
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %2 = BranchInst %BB2
//CHECK-NEXT:   %BB3:
//CHECK-NEXT:     %3 = BranchInst %BB2
//CHECK-NEXT: function_end

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz=1 -O -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -Xenable-tdz=0 -O -dump-ir %s | %FileCheck --match-full-lines %s

function outer() {
    // Ensure that foo gets inlined with or without the TDZ check.
    const foo = () => 1;
    foo();
}

// CHECK:function outer(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK-NOT: {{.*}}foo{{.*}}

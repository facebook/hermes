/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xenable-tdz -O0 -target=HBC -dump-bytecode %s | %FileCheck --match-full-lines %s

function foo() {
    return x;
    let x;
}
//CHECK-LABEL:Function<foo>{{.*}}
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    CreateEnvironment r0
//CHECK-NEXT:    LoadConstEmpty    r1
//CHECK-NEXT:    LoadConstUndefined r2
//CHECK-NEXT:    StoreToEnvironment r0, 0, r1
//CHECK-NEXT:    LoadFromEnvironment r3, r0, 0
//CHECK-NEXT:    ThrowIfEmpty      r4, r3
//CHECK-NEXT:    Ret               r4

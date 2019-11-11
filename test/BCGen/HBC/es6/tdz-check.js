/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -target=HBC -dump-bytecode %s | %FileCheck --match-full-lines %s

function foo() {
    return x;
    let x;
}
//CHECK-LABEL:Function<foo>{{.*}}
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    CreateEnvironment r0
//CHECK-NEXT:    LoadConstUndefined r1
//CHECK-NEXT:    LoadConstTrue     r2
//CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
//CHECK-NEXT:    StoreNPToEnvironment r0, 1, r1
//CHECK-NEXT:    LoadFromEnvironment r3, r0, 1
//CHECK-NEXT:    ThrowIfUndefinedInst r3
//CHECK-NEXT:    LoadFromEnvironment r4, r0, 0
//CHECK-NEXT:    Ret               r4

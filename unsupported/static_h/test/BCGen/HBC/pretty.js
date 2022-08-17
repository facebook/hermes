/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode --pretty-disassemble -O %s | %FileCheck --match-full-lines %s

function foo (a) {
    var sum = 0;
    while (--a)
        sum += a;
    print("This\nis един long Unicode string=", sum);
}

//CHECK-LABEL:Function<foo>(2 params, {{[0-9]+}} registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    LoadParam         r0, 1
//CHECK-NEXT:    Dec               r1, r0
//CHECK-NEXT:    LoadConstZero     r0
//CHECK-NEXT:    LoadConstZero     r3
//CHECK-NEXT:    JmpFalse          L1, r1
//CHECK-NEXT:L2:
//CHECK-NEXT:    Add               r0, r0, r1
//CHECK-NEXT:    Dec               r1, r1
//CHECK-NEXT:    Mov               r3, r0
//CHECK-NEXT:    JmpTrue           L2, r1
//CHECK-NEXT:L1:
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    TryGetById        r2, r0, 1, "print"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    LoadConstString   r1, "This\x0ais \u0435"...
//CHECK-NEXT:    Call3             r1, r2, r0, r1, r3
//CHECK-NEXT:    Ret               r0

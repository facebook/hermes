// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s
"use strict";

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: src 0x0, vars 0x0
//CHECK-NEXT:    DeclareGlobalVar  "foo"
//CHECK-NEXT:    CreateEnvironment r0
//CHECK-NEXT:    CreateClosure     r1, r0, 1
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    PutById           r0, r1, 1, "foo"
//CHECK-NEXT:    GetByIdShort      r3, r0, 1, "foo"
//CHECK-NEXT:    LoadConstUndefined r7
//CHECK-NEXT:    LoadConstUInt8    r4, 1
//CHECK-NEXT:    LoadConstZero     r6
//CHECK-NEXT:    LoadConstZero     r5
//CHECK-NEXT:    Call              r0, r3, 4
//CHECK-NEXT:    Ret               r0

function foo(x, y, z) { }
foo(0,0,1);

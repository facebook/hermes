/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -target=HBC -dump-bytecode --basic-block-profiling -O %s | %FileCheck --match-full-lines %s

var condition = false;
try {
  try {
    print(condition? "yes": "no");
  } finally {
    print("rethrowing");
  }
} catch (e) {
  print(e.stack);
}

//CHECK-LABEL:Function<global>{{.*}}
//CHECK-NEXT:Offset in debug table:{{.*}}
//CHECK-NEXT:    DeclareGlobalVar  "condition"
//CHECK-NEXT:    ProfilePoint      10
//CHECK-NEXT:    LoadConstString   r5, "yes"
//CHECK-NEXT:    LoadConstString   r4, "no"
//CHECK-NEXT:    LoadConstUndefined r3
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    LoadConstFalse    r1
//CHECK-NEXT:    GetGlobalObject   r2
//CHECK-NEXT:    PutById           r2, r1, 1, "condition"
//CHECK-NEXT:L8:
//CHECK-NEXT:    ProfilePoint      7
//CHECK-NEXT:L6:
//CHECK-NEXT:    ProfilePoint      5
//CHECK-NEXT:    TryGetById        r1, r2, 1, "print"
//CHECK-NEXT:    GetByIdShort      r6, r2, 2, "condition"
//CHECK-NEXT:    JmpFalse          L1, r6
//CHECK-NEXT:    ProfilePoint      4
//CHECK-NEXT:    Mov               r4, r5
//CHECK-NEXT:L1:
//CHECK-NEXT:    ProfilePoint      3
//CHECK-NEXT:    Call2             r0, r1, r3, r4
//CHECK-NEXT:L7:
//CHECK-NEXT:    ProfilePoint      2
//CHECK-NEXT:    TryGetById        r4, r2, 1, "print"
//CHECK-NEXT:    LoadConstString   r1, "rethrowing"
//CHECK-NEXT:    Call2             r0, r4, r3, r1
//CHECK-NEXT:L9:
//CHECK-NEXT:    ProfilePoint      1
//CHECK-NEXT:    Jmp               L3
//CHECK-NEXT:L2:
//CHECK-NEXT:    Catch             r1
//CHECK-NEXT:    ProfilePoint      6
//CHECK-NEXT:    TryGetById        r5, r2, 1, "print"
//CHECK-NEXT:    LoadConstString   r4, "rethrowing"
//CHECK-NEXT:    Call2             r0, r5, r3, r4
//CHECK-NEXT:    Throw             r1
//CHECK-NEXT:L4:
//CHECK-NEXT:    Catch             r1
//CHECK-NEXT:    ProfilePoint      9
//CHECK-NEXT:    TryGetById        r2, r2, 1, "print"
//CHECK-NEXT:    GetByIdShort      r1, r1, 3, "stack"
//CHECK-NEXT:    Call2             r0, r2, r3, r1
//CHECK-NEXT:L3:
//CHECK-NEXT:    ProfilePoint      8
//CHECK-NEXT:    Ret               r0

//CHECK-LABEL:Exception Handlers:
//CHECK-NEXT:0: start = L6, end = L7, target = L2
//CHECK-NEXT:1: start = L8, end = L9, target = L4
//CHECK-NEXT:2: start = L2, end = L4, target = L4

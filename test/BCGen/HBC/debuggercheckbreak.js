/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -target=HBC %s -O -g | %FileCheck %s --match-full-lines

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHECK-NEXT:    DeclareGlobalVar  "test1"
//CHECK-NEXT:    CreateEnvironment r0
//CHECK-NEXT:    CreateClosure     r1, r0, 1
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    PutById           r0, r1, 1, "test1"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    AsyncBreakCheck
//CHECK-NEXT:    Ret               r0

//CHECK-LABEL:Function<test1>(1 params, 16 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    LoadConstUInt8    r4, 3
//CHECK-NEXT:    LoadConstUInt8    r1, 5
//CHECK-NEXT:    LoadConstUInt8    r3, 10
//CHECK-NEXT:    LoadConstZero     r5
//CHECK-NEXT:    AsyncBreakCheck
//CHECK-NEXT:L3:
//CHECK-NEXT:    TryGetById        r6, r0, 1, "Math"
//CHECK-NEXT:    GetByIdShort      r2, r6, 2, "random"
//CHECK-NEXT:    Call1             r6, r2, r6
//CHECK-NEXT:    Mov               r2, r5
//CHECK-NEXT:    AsyncBreakCheck
//CHECK-NEXT:    JStrictEqual      L1, r6, r4
//CHECK-NEXT:    TryGetById        r7, r0, 1, "Math"
//CHECK-NEXT:    GetByIdShort      r6, r7, 2, "random"
//CHECK-NEXT:    Call1             r6, r6, r7
//CHECK-NEXT:    JStrictEqual      L2, r6, r1
//CHECK-NEXT:    Inc               r5, r2
//CHECK-NEXT:    Jmp               L3
//CHECK-NEXT:L2:
//CHECK-NEXT:    AsyncBreakCheck
//CHECK-NEXT:    Jmp               L2
//CHECK-NEXT:L1:
//CHECK-NEXT:    Mov               r1, r2
//CHECK-NEXT:    Mov               r2, r1
//CHECK-NEXT:    JNotGreater       L4, r2, r3
//CHECK-NEXT:L5:
//CHECK-NEXT:    Dec               r1, r1
//CHECK-NEXT:    Mov               r2, r1
//CHECK-NEXT:    AsyncBreakCheck
//CHECK-NEXT:    JGreater          L5, r2, r3
//CHECK-NEXT:L4:
//CHECK-NEXT:    TryGetById        r1, r0, 3, "print"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    Call2             r1, r1, r0, r2
//CHECK-NEXT:    Ret               r0

function test1() {
  var count = 0;
  for(var count=0; ; count++) {
    if (Math.random() === 3)
      break;
    if (Math.random() === 5)
      for(;;){} // infinite loop
  }
  while (count > 10)
    count--;
  print(count);
}

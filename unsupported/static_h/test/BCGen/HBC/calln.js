/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-lra %s | %FileCheck --check-prefix=LRA --match-full-lines %s
// RUN: %hermes -O -dump-bytecode %s | %FileCheck --check-prefix=BCGEN --match-full-lines %s

// Variants that should produce a HBCCallNInst instruction.

function foo1(f) { f(); }
// LRA: function foo1(f) : undefined
// LRA-NEXT: frame = []
// LRA-NEXT: %BB0:
// LRA-NEXT:   $Reg1 @0 [1...3) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:   $Reg0 @1 [2...4) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:   $Reg2           	%2 = ImplicitMovInst %1 : undefined
// LRA-NEXT:   $Reg1 @2 [empty]	%3 = HBCCallNInst %0, %1 : undefined
// LRA-NEXT:   $Reg0 @3 [empty]	%4 = ReturnInst %1 : undefined
// LRA-NEXT: function_end

// BCGEN: Function<foo1>{{.*}}
// BCGEN-NEXT: Offset in debug table: {{.*}}
// BCGEN-NEXT:     LoadParam         r1, 1
// BCGEN-NEXT:     LoadConstUndefined r0
// BCGEN-NEXT:     Call1             r1, r1, r0
// BCGEN-NEXT:     Ret               r0


function foo2(f) { f(1); }
// LRA: function foo2(f) : undefined
// LRA-NEXT: frame = []
// LRA-NEXT: %BB0:
// LRA-NEXT:   $Reg2 @0 [1...4) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:   $Reg0 @1 [2...5) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:   $Reg1 @2 [3...4) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:   $Reg4           	%3 = ImplicitMovInst %1 : undefined
// LRA-NEXT:   $Reg3           	%4 = ImplicitMovInst %2 : number
// LRA-NEXT:   $Reg1 @3 [empty]	%5 = HBCCallNInst %0, %1 : undefined, %2 : number
// LRA-NEXT:   $Reg0 @4 [empty]	%6 = ReturnInst %1 : undefined
// LRA-NEXT: function_end

// BCGEN: Function<foo2>{{.*}}
// BCGEN-NEXT: Offset in debug table: {{.*}}
// BCGEN-NEXT:     LoadParam         r2, 1
// BCGEN-NEXT:     LoadConstUndefined r0
// BCGEN-NEXT:     LoadConstUInt8    r1, 1
// BCGEN-NEXT:     Call2             r1, r2, r0, r1
// BCGEN-NEXT:     Ret               r0


function foo3(f) { f(1, 2); }
// LRA: function foo3(f) : undefined
// LRA-NEXT: frame = []
// LRA-NEXT: %BB0:
// LRA-NEXT:   $Reg3 @0 [1...5) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:   $Reg0 @1 [2...6) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:   $Reg2 @2 [3...5) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:   $Reg1 @3 [4...5) 	%3 = HBCLoadConstInst 2 : number
// LRA-NEXT:   $Reg6           	%4 = ImplicitMovInst %1 : undefined
// LRA-NEXT:   $Reg5           	%5 = ImplicitMovInst %2 : number
// LRA-NEXT:   $Reg4           	%6 = ImplicitMovInst %3 : number
// LRA-NEXT:   $Reg1 @4 [empty]	%7 = HBCCallNInst %0, %1 : undefined, %2 : number, %3 : number
// LRA-NEXT:   $Reg0 @5 [empty]	%8 = ReturnInst %1 : undefined
// LRA-NEXT: function_end

// BCGEN: Function<foo3>{{.*}}
// BCGEN-NEXT: Offset in debug table: {{.*}}
// BCGEN-NEXT:     LoadParam         r3, 1
// BCGEN-NEXT:     LoadConstUndefined r0
// BCGEN-NEXT:     LoadConstUInt8    r2, 1
// BCGEN-NEXT:     LoadConstUInt8    r1, 2
// BCGEN-NEXT:     Call3             r1, r3, r0, r2, r1
// BCGEN-NEXT:     Ret               r0



function foo4(f) { f(1, 2, 3); }
// LRA: function foo4(f) : undefined
// LRA-NEXT: frame = []
// LRA-NEXT: %BB0:
// LRA-NEXT:   $Reg4 @0 [1...6) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:   $Reg0 @1 [2...7) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:   $Reg3 @2 [3...6) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:   $Reg2 @3 [4...6) 	%3 = HBCLoadConstInst 2 : number
// LRA-NEXT:   $Reg1 @4 [5...6) 	%4 = HBCLoadConstInst 3 : number
// LRA-NEXT:   $Reg8           	%5 = ImplicitMovInst %1 : undefined
// LRA-NEXT:   $Reg7           	%6 = ImplicitMovInst %2 : number
// LRA-NEXT:   $Reg6           	%7 = ImplicitMovInst %3 : number
// LRA-NEXT:   $Reg5           	%8 = ImplicitMovInst %4 : number
// LRA-NEXT:   $Reg1 @5 [empty]	%9 = HBCCallNInst %0, %1 : undefined, %2 : number, %3 : number, %4 : number
// LRA-NEXT:   $Reg0 @6 [empty]	%10 = ReturnInst %1 : undefined
// LRA-NEXT: function_end

// BCGEN: Function<foo4>{{.*}}
// BCGEN-NEXT: Offset in debug table: {{.*}}
// BCGEN-NEXT:     LoadParam         r4, 1
// BCGEN-NEXT:     LoadConstUndefined r0
// BCGEN-NEXT:     LoadConstUInt8    r3, 1
// BCGEN-NEXT:     LoadConstUInt8    r2, 2
// BCGEN-NEXT:     LoadConstUInt8    r1, 3
// BCGEN-NEXT:     Call4             r1, r4, r0, r3, r2, r1
// BCGEN-NEXT:     Ret               r0


// This has too many parameters and so will be an ordinary Call instruction, not HBCCallNInst.

function foo5(f) { f(1, 2, 3, 4); }
// LRA: function foo5(f) : undefined
// LRA-NEXT: frame = []
// LRA-NEXT: %BB0:
// LRA-NEXT:   $Reg5 @0 [1...7) 	%0 = HBCLoadParamInst 1 : number
// LRA-NEXT:   $Reg0 @1 [2...8) 	%1 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:   $Reg9 @2 [3...7) 	%2 = HBCLoadConstInst 1 : number
// LRA-NEXT:   $Reg8 @3 [4...7) 	%3 = HBCLoadConstInst 2 : number
// LRA-NEXT:   $Reg7 @4 [5...7) 	%4 = HBCLoadConstInst 3 : number
// LRA-NEXT:   $Reg6 @5 [6...7) 	%5 = HBCLoadConstInst 4 : number
// LRA-NEXT:   $Reg10           	%6 = HBCLoadConstInst undefined : undefined
// LRA-NEXT:   $Reg1 @6 [empty]	%7 = CallInst %0, %6 : undefined, %2 : number, %3 : number, %4 : number, %5 : number
// LRA-NEXT:   $Reg0 @7 [empty]	%8 = ReturnInst %1 : undefined
// LRA-NEXT: function_end

// BCGEN: Function<foo5>{{.*}}
// BCGEN-NEXT: Offset in debug table: {{.*}}
// BCGEN-NEXT:     LoadParam         r5, 1
// BCGEN-NEXT:     LoadConstUndefined r0
// BCGEN-NEXT:     LoadConstUInt8    r9, 1
// BCGEN-NEXT:     LoadConstUInt8    r8, 2
// BCGEN-NEXT:     LoadConstUInt8    r7, 3
// BCGEN-NEXT:     LoadConstUInt8    r6, 4
// BCGEN-NEXT:     LoadConstUndefined r10
// BCGEN-NEXT:     Call              r1, r5, 5
// BCGEN-NEXT:     Ret               r0

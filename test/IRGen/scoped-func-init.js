/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -dump-ir %s | %FileCheckOrRegen --check-prefix=CHKLOOSE --match-full-lines %s
// RUN: %shermes -O0 -dump-ir -strict %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKSTRICT %s

// Test that the variable `f` is initialized to undefined at the start of the
// scope but only in loose mode.

var init, f;

function foo() {
  init = f;
  {
    function f() {}
  }
}

// Auto-generated content below. Please do not modify manually.

// CHKLOOSE:function global(): any
// CHKLOOSE-NEXT:frame = []
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:  %0 = DeclareGlobalVarInst "init": string
// CHKLOOSE-NEXT:  %1 = DeclareGlobalVarInst "f": string
// CHKLOOSE-NEXT:  %2 = DeclareGlobalVarInst "foo": string
// CHKLOOSE-NEXT:  %3 = CreateFunctionInst (:closure) %foo(): any
// CHKLOOSE-NEXT:  %4 = StorePropertyLooseInst %3: closure, globalObject: object, "foo": string
// CHKLOOSE-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHKLOOSE-NEXT:  %6 = StoreStackInst undefined: undefined, %5: any
// CHKLOOSE-NEXT:  %7 = LoadStackInst (:any) %5: any
// CHKLOOSE-NEXT:  %8 = ReturnInst %7: any
// CHKLOOSE-NEXT:function_end

// CHKLOOSE:function foo(): any
// CHKLOOSE-NEXT:frame = [f: any]
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:  %0 = StoreFrameInst undefined: undefined, [f]: any
// CHKLOOSE-NEXT:  %1 = LoadFrameInst (:any) [f]: any
// CHKLOOSE-NEXT:  %2 = StorePropertyLooseInst %1: any, globalObject: object, "init": string
// CHKLOOSE-NEXT:  %3 = CreateFunctionInst (:closure) %f(): any
// CHKLOOSE-NEXT:  %4 = StoreFrameInst %3: closure, [f]: any
// CHKLOOSE-NEXT:  %5 = ReturnInst undefined: undefined
// CHKLOOSE-NEXT:function_end

// CHKLOOSE:function f(): any
// CHKLOOSE-NEXT:frame = []
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:  %0 = ReturnInst undefined: undefined
// CHKLOOSE-NEXT:function_end

// CHKSTRICT:function global(): any
// CHKSTRICT-NEXT:frame = []
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:  %0 = DeclareGlobalVarInst "init": string
// CHKSTRICT-NEXT:  %1 = DeclareGlobalVarInst "f": string
// CHKSTRICT-NEXT:  %2 = DeclareGlobalVarInst "foo": string
// CHKSTRICT-NEXT:  %3 = CreateFunctionInst (:closure) %foo(): any
// CHKSTRICT-NEXT:  %4 = StorePropertyStrictInst %3: closure, globalObject: object, "foo": string
// CHKSTRICT-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHKSTRICT-NEXT:  %6 = StoreStackInst undefined: undefined, %5: any
// CHKSTRICT-NEXT:  %7 = LoadStackInst (:any) %5: any
// CHKSTRICT-NEXT:  %8 = ReturnInst %7: any
// CHKSTRICT-NEXT:function_end

// CHKSTRICT:function foo(): any
// CHKSTRICT-NEXT:frame = [f: any]
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:  %0 = LoadPropertyInst (:any) globalObject: object, "f": string
// CHKSTRICT-NEXT:  %1 = StorePropertyStrictInst %0: any, globalObject: object, "init": string
// CHKSTRICT-NEXT:  %2 = CreateFunctionInst (:closure) %f(): any
// CHKSTRICT-NEXT:  %3 = StoreFrameInst %2: closure, [f]: any
// CHKSTRICT-NEXT:  %4 = ReturnInst undefined: undefined
// CHKSTRICT-NEXT:function_end

// CHKSTRICT:function f(): any
// CHKSTRICT-NEXT:frame = []
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:  %0 = ReturnInst undefined: undefined
// CHKSTRICT-NEXT:function_end

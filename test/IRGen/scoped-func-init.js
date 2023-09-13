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
// CHKLOOSE-NEXT:       DeclareGlobalVarInst "init": string
// CHKLOOSE-NEXT:       DeclareGlobalVarInst "f": string
// CHKLOOSE-NEXT:       DeclareGlobalVarInst "foo": string
// CHKLOOSE-NEXT:  %3 = CreateFunctionInst (:object) %foo(): any
// CHKLOOSE-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "foo": string
// CHKLOOSE-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHKLOOSE-NEXT:       StoreStackInst undefined: undefined, %5: any
// CHKLOOSE-NEXT:  %7 = LoadStackInst (:any) %5: any
// CHKLOOSE-NEXT:       ReturnInst %7: any
// CHKLOOSE-NEXT:function_end

// CHKLOOSE:function foo(): any
// CHKLOOSE-NEXT:frame = [f: any]
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:       StoreFrameInst undefined: undefined, [f]: any
// CHKLOOSE-NEXT:  %1 = LoadFrameInst (:any) [f]: any
// CHKLOOSE-NEXT:       StorePropertyLooseInst %1: any, globalObject: object, "init": string
// CHKLOOSE-NEXT:  %3 = CreateFunctionInst (:object) %f(): any
// CHKLOOSE-NEXT:       StoreFrameInst %3: object, [f]: any
// CHKLOOSE-NEXT:       ReturnInst undefined: undefined
// CHKLOOSE-NEXT:function_end

// CHKLOOSE:function f(): any
// CHKLOOSE-NEXT:frame = []
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:       ReturnInst undefined: undefined
// CHKLOOSE-NEXT:function_end

// CHKSTRICT:function global(): any
// CHKSTRICT-NEXT:frame = []
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:       DeclareGlobalVarInst "init": string
// CHKSTRICT-NEXT:       DeclareGlobalVarInst "f": string
// CHKSTRICT-NEXT:       DeclareGlobalVarInst "foo": string
// CHKSTRICT-NEXT:  %3 = CreateFunctionInst (:object) %foo(): any
// CHKSTRICT-NEXT:       StorePropertyStrictInst %3: object, globalObject: object, "foo": string
// CHKSTRICT-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHKSTRICT-NEXT:       StoreStackInst undefined: undefined, %5: any
// CHKSTRICT-NEXT:  %7 = LoadStackInst (:any) %5: any
// CHKSTRICT-NEXT:       ReturnInst %7: any
// CHKSTRICT-NEXT:function_end

// CHKSTRICT:function foo(): any
// CHKSTRICT-NEXT:frame = [f: any]
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:  %0 = LoadPropertyInst (:any) globalObject: object, "f": string
// CHKSTRICT-NEXT:       StorePropertyStrictInst %0: any, globalObject: object, "init": string
// CHKSTRICT-NEXT:  %2 = CreateFunctionInst (:object) %f(): any
// CHKSTRICT-NEXT:       StoreFrameInst %2: object, [f]: any
// CHKSTRICT-NEXT:       ReturnInst undefined: undefined
// CHKSTRICT-NEXT:function_end

// CHKSTRICT:function f(): any
// CHKSTRICT-NEXT:frame = []
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:       ReturnInst undefined: undefined
// CHKSTRICT-NEXT:function_end

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

// CHKLOOSE:scope %VS0 []

// CHKLOOSE:function global(): any
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKLOOSE-NEXT:       DeclareGlobalVarInst "init": string
// CHKLOOSE-NEXT:       DeclareGlobalVarInst "f": string
// CHKLOOSE-NEXT:       DeclareGlobalVarInst "foo": string
// CHKLOOSE-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHKLOOSE-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHKLOOSE-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHKLOOSE-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHKLOOSE-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHKLOOSE-NEXT:       ReturnInst %8: any
// CHKLOOSE-NEXT:function_end

// CHKLOOSE:scope %VS1 [f: any, f#1: any]

// CHKLOOSE:function foo(): any
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKLOOSE-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHKLOOSE-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.f]: any
// CHKLOOSE-NEXT:  %3 = LoadFrameInst (:any) %1: environment, [%VS1.f]: any
// CHKLOOSE-NEXT:       StorePropertyLooseInst %3: any, globalObject: object, "init": string
// CHKLOOSE-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHKLOOSE-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.f]: any
// CHKLOOSE-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.f#1]: any
// CHKLOOSE-NEXT:       ReturnInst undefined: undefined
// CHKLOOSE-NEXT:function_end

// CHKLOOSE:scope %VS2 []

// CHKLOOSE:function f(): any
// CHKLOOSE-NEXT:%BB0:
// CHKLOOSE-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHKLOOSE-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHKLOOSE-NEXT:       ReturnInst undefined: undefined
// CHKLOOSE-NEXT:function_end

// CHKSTRICT:scope %VS0 []

// CHKSTRICT:function global(): any
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKSTRICT-NEXT:       DeclareGlobalVarInst "init": string
// CHKSTRICT-NEXT:       DeclareGlobalVarInst "f": string
// CHKSTRICT-NEXT:       DeclareGlobalVarInst "foo": string
// CHKSTRICT-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHKSTRICT-NEXT:       StorePropertyStrictInst %4: object, globalObject: object, "foo": string
// CHKSTRICT-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHKSTRICT-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHKSTRICT-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHKSTRICT-NEXT:       ReturnInst %8: any
// CHKSTRICT-NEXT:function_end

// CHKSTRICT:scope %VS1 [f: any]

// CHKSTRICT:function foo(): any
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKSTRICT-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHKSTRICT-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "f": string
// CHKSTRICT-NEXT:       StorePropertyStrictInst %2: any, globalObject: object, "init": string
// CHKSTRICT-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHKSTRICT-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.f]: any
// CHKSTRICT-NEXT:       ReturnInst undefined: undefined
// CHKSTRICT-NEXT:function_end

// CHKSTRICT:scope %VS2 []

// CHKSTRICT:function f(): any
// CHKSTRICT-NEXT:%BB0:
// CHKSTRICT-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHKSTRICT-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHKSTRICT-NEXT:       ReturnInst undefined: undefined
// CHKSTRICT-NEXT:function_end

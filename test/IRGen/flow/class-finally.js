/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Ensure there's only one copy of the C constructor.
try {} finally { class C {} }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, C: any, ?C.prototype: object, ?C.prototype#1: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [C]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [C]: any
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [?C.prototype#1]: object
// CHECK-NEXT:        StorePropertyStrictInst %9: object, %7: object, "prototype": string
// CHECK-NEXT:        ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [C]: any
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %1: environment, %C(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: object, [C]: any
// CHECK-NEXT:  %19 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: object, [?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %19: object, %17: object, "prototype": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

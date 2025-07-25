/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s -dump-ir | %FileCheckOrRegen --match-full-lines %s

(function() {
  try {
  } catch (e) {
  } finally {
    class C1 {
      f1 = 10;
      static f1 = 10;
    };
  }
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  %4 = CallInst (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreStackInst %4: any, %1: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [e: any, C1: any, C1#1: any, ?C1.prototype: object, ?C1: object, <instElemInitFunc:C1>: object, ?C1.prototype#1: object, ?C1#1: object, <instElemInitFunc:C1>#1: object]

// CHECK:function ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C1#1]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<instance_members_initializer:C1>(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.<instElemInitFunc:C1>#1]: object
// CHECK-NEXT:  %8 = AllocStackInst (:object) $?anon_1_clsPrototype: any
// CHECK-NEXT:  %9 = CreateClassInst (:object) %1: environment, %VS1: any, %C1(): functionCode, empty: any, %8: object
// CHECK-NEXT:  %10 = LoadStackInst (:object) %8: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.C1#1]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.?C1#1]: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS1.?C1.prototype#1]: object
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<static_elements_initializer:C1>(): functionCode
// CHECK-NEXT:  %15 = CallInst (:any) %14: object, %<static_elements_initializer:C1>(): functionCode, true: boolean, %1: environment, undefined: undefined, %9: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.C1]: any
// CHECK-NEXT:        ThrowInst %3: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %20 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: any, [%VS1.e]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst %BB1, %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst %BB4, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [%VS1.C1]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [%VS1.C1#1]: any
// CHECK-NEXT:  %28 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<instance_members_initializer:C1>(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %28: object, [%VS1.<instElemInitFunc:C1>]: object
// CHECK-NEXT:  %30 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %31 = CreateClassInst (:object) %1: environment, %VS1: any, %C1(): functionCode, empty: any, %30: object
// CHECK-NEXT:  %32 = LoadStackInst (:object) %30: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: object, [%VS1.C1#1]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: object, [%VS1.?C1]: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %32: object, [%VS1.?C1.prototype]: object
// CHECK-NEXT:  %36 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<static_elements_initializer:C1>(): functionCode
// CHECK-NEXT:  %37 = CallInst (:any) %36: object, %<static_elements_initializer:C1>(): functionCode, true: boolean, %1: environment, undefined: undefined, %31: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: object, [%VS1.C1]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function <instance_members_initializer:C1>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS2: any, %1: environment
// CHECK-NEXT:       DefineOwnPropertyInst 10: number, %0: any, "f1": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [this: object]

// CHECK:base constructor C1(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "prototype": string
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) %3: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS3.this]: object
// CHECK-NEXT:  %6 = LoadFrameInst (:object) %0: environment, [%VS1.<instElemInitFunc:C1>]: object
// CHECK-NEXT:  %7 = LoadFrameInst (:object) %1: environment, [%VS3.this]: object
// CHECK-NEXT:  %8 = CallInst (:undefined) %6: object, empty: any, true: boolean, empty: any, undefined: undefined, %7: object
// CHECK-NEXT:  %9 = LoadFrameInst (:object) %1: environment, [%VS3.this]: object
// CHECK-NEXT:        ReturnInst %9: object
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:function <static_elements_initializer:C1>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS1.?C1]: object
// CHECK-NEXT:       DefineOwnPropertyInst 10: number, %2: object, "f1": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

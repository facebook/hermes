/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s -dump-ir | %FileCheckOrRegen --match-full-lines %s

// Regression test: class with static property in finally block.
// The finally block causes code to be emitted twice. Previously,
// internal class variables were created fresh on each compilation,
// but initializer functions were cached. This caused the static
// initializer to load from an uninitialized variable, crashing at
// runtime with "PutOwn requires object operand".

function main() {
    try {
        throw 1;
    } finally {
        class C5 {
            static g = "-10238";
        }
    }
}
main();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "main": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "main": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [C5: any, C5#1: any, ?C5.prototype: object, ?C5: object]

// CHECK:function main(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C5]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C5#1]: any
// CHECK-NEXT:  %6 = AllocStackInst (:object) $?anon_1_clsPrototype: any
// CHECK-NEXT:  %7 = CreateClassInst (:object) %1: environment, %VS1: any, %C5(): functionCode, empty: any, %6: object
// CHECK-NEXT:  %8 = LoadStackInst (:object) %6: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [%VS1.C5#1]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %7: object, [%VS1.?C5]: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %8: object, [%VS1.?C5.prototype]: object
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<static_elements_initializer:C5>(): functionCode
// CHECK-NEXT:  %13 = CallInst (:any) %12: object, %<static_elements_initializer:C5>(): functionCode, true: boolean, %1: environment, undefined: undefined, %7: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %7: object, [%VS1.C5]: any
// CHECK-NEXT:        ThrowInst %3: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ThrowInst 1: number, %BB1
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [this: object]

// CHECK:base constructor C5(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "prototype": string
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) %3: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS2.this]: object
// CHECK-NEXT:  %6 = LoadFrameInst (:object) %1: environment, [%VS2.this]: object
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function <static_elements_initializer:C5>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS1.?C5]: object
// CHECK-NEXT:       DefineOwnPropertyInst "-10238": string, %2: object, "g": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

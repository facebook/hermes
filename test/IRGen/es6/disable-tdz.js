/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 --test262 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %shermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKDIS %s

function check1() {
    glob = function inner() {
        ++x;
        return y;
    }
    return x + y;
    let x = 10;
    const y = 1;
    return x + y;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "check1": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %check1(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "check1": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function check1(): any
// CHECK-NEXT:frame = [x: any|empty, y: any|empty, inner: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst empty: empty, [x]: any|empty
// CHECK-NEXT:       StoreFrameInst empty: empty, [y]: any|empty
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %inner(): any
// CHECK-NEXT:       StoreFrameInst %2: object, [inner]: any
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "glob": string
// CHECK-NEXT:  %5 = ThrowIfEmptyInst (:any) empty: empty
// CHECK-NEXT:  %6 = ThrowIfEmptyInst (:any) empty: empty
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %5: any, %6: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function inner(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [x@check1]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:  %2 = UnaryIncInst (:any) %1: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any|empty) [x@check1]: any|empty
// CHECK-NEXT:  %4 = ThrowIfEmptyInst (:any) %3: any|empty
// CHECK-NEXT:       StoreFrameInst %2: any, [x@check1]: any|empty
// CHECK-NEXT:  %6 = LoadFrameInst (:any|empty) [y@check1]: any|empty
// CHECK-NEXT:  %7 = ThrowIfEmptyInst (:any) %6: any|empty
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHKDIS:function global(): any
// CHKDIS-NEXT:frame = []
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:       DeclareGlobalVarInst "check1": string
// CHKDIS-NEXT:  %1 = CreateFunctionInst (:object) %check1(): any
// CHKDIS-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "check1": string
// CHKDIS-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKDIS-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKDIS-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKDIS-NEXT:       ReturnInst %5: any
// CHKDIS-NEXT:function_end

// CHKDIS:function check1(): any
// CHKDIS-NEXT:frame = [x: any, y: any, inner: any]
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHKDIS-NEXT:       StoreFrameInst undefined: undefined, [y]: any
// CHKDIS-NEXT:  %2 = CreateFunctionInst (:object) %inner(): any
// CHKDIS-NEXT:       StoreFrameInst %2: object, [inner]: any
// CHKDIS-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "glob": string
// CHKDIS-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHKDIS-NEXT:  %6 = LoadFrameInst (:any) [y]: any
// CHKDIS-NEXT:  %7 = BinaryAddInst (:any) %5: any, %6: any
// CHKDIS-NEXT:       ReturnInst %7: any
// CHKDIS-NEXT:function_end

// CHKDIS:function inner(): any
// CHKDIS-NEXT:frame = []
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = LoadFrameInst (:any) [x@check1]: any
// CHKDIS-NEXT:  %1 = UnaryIncInst (:any) %0: any
// CHKDIS-NEXT:       StoreFrameInst %1: any, [x@check1]: any
// CHKDIS-NEXT:  %3 = LoadFrameInst (:any) [y@check1]: any
// CHKDIS-NEXT:       ReturnInst %3: any
// CHKDIS-NEXT:function_end

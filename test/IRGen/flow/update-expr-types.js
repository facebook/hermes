/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=f1,f2,f3,f4,f5,f6 -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that the type of the lvalue load is correct and the type of the whole
// expression is correct.

function f1(n: number): number {
    return ++n;
}
function f2(n: number): number {
    return n++;
}
function f3(n: any): any {
    return ++n;
}
function f4(n: any): any {
    return n++;
}
function f5(n: number): number {
    return n += 10;
}
function f6(n: number, a: any): number {
    return n += a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function f1(n: number): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [n]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:  %6 = UnaryIncInst (:number) %5: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number, [n]: any
// CHECK-NEXT:       ReturnInst %6: number
// CHECK-NEXT:function_end

// CHECK:function f2(n: number): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [n]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:  %6 = UnaryIncInst (:number) %5: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number, [n]: any
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function f3(n: any): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f3(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [n]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %5 = UnaryIncInst (:number|bigint) %4: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: number|bigint, [n]: any
// CHECK-NEXT:       ReturnInst %5: number|bigint
// CHECK-NEXT:function_end

// CHECK:function f4(n: any): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f4(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [n]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %5 = AsNumericInst (:number|bigint) %4: any
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: number|bigint
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number|bigint, [n]: any
// CHECK-NEXT:       ReturnInst %5: number|bigint
// CHECK-NEXT:function_end

// CHECK:function f5(n: number): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f5(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [n]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %5: number, 10: number
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any, type(number)
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: number, [n]: any
// CHECK-NEXT:       ReturnInst %7: number
// CHECK-NEXT:function_end

// CHECK:function f6(n: number, a: any): any [typed]
// CHECK-NEXT:frame = [n: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f6(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [n]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [a]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any, type(number)
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: number, %8: any
// CHECK-NEXT:  %10 = CheckedTypeCastInst (:number) %9: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: number, [n]: any
// CHECK-NEXT:        ReturnInst %10: number
// CHECK-NEXT:function_end

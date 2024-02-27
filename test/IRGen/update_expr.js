/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function update_field_test0(o) { return o.f++; }

function update_field_test1(o) { return o.f--; }

function update_field_test2(o) { return ++o.f; }

function update_field_test3(o) { return --o.f; }

function update_variable_test0(x) { return x++; }

function update_variable_test1(x) { return x--; }

function update_variable_test2(x) { return ++x; }

function update_variable_test3(x) { return --x; }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "update_field_test0": string
// CHECK-NEXT:       DeclareGlobalVarInst "update_field_test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "update_field_test2": string
// CHECK-NEXT:       DeclareGlobalVarInst "update_field_test3": string
// CHECK-NEXT:       DeclareGlobalVarInst "update_variable_test0": string
// CHECK-NEXT:       DeclareGlobalVarInst "update_variable_test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "update_variable_test2": string
// CHECK-NEXT:       DeclareGlobalVarInst "update_variable_test3": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %update_field_test0(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "update_field_test0": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %update_field_test1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "update_field_test1": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %update_field_test2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "update_field_test2": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %update_field_test3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "update_field_test3": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %update_variable_test0(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "update_variable_test0": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %0: environment, %update_variable_test1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "update_variable_test1": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %0: environment, %update_variable_test2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "update_variable_test2": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %0: environment, %update_variable_test3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "update_variable_test3": string
// CHECK-NEXT:  %25 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %25: any
// CHECK-NEXT:  %27 = LoadStackInst (:any) %25: any
// CHECK-NEXT:        ReturnInst %27: any
// CHECK-NEXT:function_end

// CHECK:function update_field_test0(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_field_test0(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [o]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "f": string
// CHECK-NEXT:  %6 = AsNumericInst (:number|bigint) %5: any
// CHECK-NEXT:  %7 = UnaryIncInst (:number|bigint) %6: number|bigint
// CHECK-NEXT:       StorePropertyLooseInst %7: number|bigint, %4: any, "f": string
// CHECK-NEXT:       ReturnInst %6: number|bigint
// CHECK-NEXT:function_end

// CHECK:function update_field_test1(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_field_test1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [o]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "f": string
// CHECK-NEXT:  %6 = AsNumericInst (:number|bigint) %5: any
// CHECK-NEXT:  %7 = UnaryDecInst (:number|bigint) %6: number|bigint
// CHECK-NEXT:       StorePropertyLooseInst %7: number|bigint, %4: any, "f": string
// CHECK-NEXT:       ReturnInst %6: number|bigint
// CHECK-NEXT:function_end

// CHECK:function update_field_test2(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_field_test2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [o]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "f": string
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: any
// CHECK-NEXT:       StorePropertyLooseInst %6: number|bigint, %4: any, "f": string
// CHECK-NEXT:       ReturnInst %6: number|bigint
// CHECK-NEXT:function_end

// CHECK:function update_field_test3(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_field_test3(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [o]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "f": string
// CHECK-NEXT:  %6 = UnaryDecInst (:number|bigint) %5: any
// CHECK-NEXT:       StorePropertyLooseInst %6: number|bigint, %4: any, "f": string
// CHECK-NEXT:       ReturnInst %6: number|bigint
// CHECK-NEXT:function_end

// CHECK:function update_variable_test0(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_variable_test0(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = AsNumericInst (:number|bigint) %4: any
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: number|bigint
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number|bigint, [x]: any
// CHECK-NEXT:       ReturnInst %5: number|bigint
// CHECK-NEXT:function_end

// CHECK:function update_variable_test1(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_variable_test1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = AsNumericInst (:number|bigint) %4: any
// CHECK-NEXT:  %6 = UnaryDecInst (:number|bigint) %5: number|bigint
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number|bigint, [x]: any
// CHECK-NEXT:       ReturnInst %5: number|bigint
// CHECK-NEXT:function_end

// CHECK:function update_variable_test2(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_variable_test2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = UnaryIncInst (:number|bigint) %4: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: number|bigint, [x]: any
// CHECK-NEXT:       ReturnInst %5: number|bigint
// CHECK-NEXT:function_end

// CHECK:function update_variable_test3(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %update_variable_test3(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = UnaryDecInst (:number|bigint) %4: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: number|bigint, [x]: any
// CHECK-NEXT:       ReturnInst %5: number|bigint
// CHECK-NEXT:function_end

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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "update_field_test0": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "update_field_test1": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "update_field_test2": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "update_field_test3": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "update_variable_test0": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "update_variable_test1": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "update_variable_test2": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "update_variable_test3": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %update_field_test0(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: object, globalObject: object, "update_field_test0": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %update_field_test1(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: object, globalObject: object, "update_field_test1": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %update_field_test2(): any
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12: object, globalObject: object, "update_field_test2": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %update_field_test3(): any
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14: object, globalObject: object, "update_field_test3": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %update_variable_test0(): any
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16: object, globalObject: object, "update_variable_test0": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %update_variable_test1(): any
// CHECK-NEXT:  %19 = StorePropertyLooseInst %18: object, globalObject: object, "update_variable_test1": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %update_variable_test2(): any
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20: object, globalObject: object, "update_variable_test2": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %update_variable_test3(): any
// CHECK-NEXT:  %23 = StorePropertyLooseInst %22: object, globalObject: object, "update_variable_test3": string
// CHECK-NEXT:  %24 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %25 = StoreStackInst undefined: undefined, %24: any
// CHECK-NEXT:  %26 = LoadStackInst (:any) %24: any
// CHECK-NEXT:  %27 = ReturnInst %26: any
// CHECK-NEXT:function_end

// CHECK:function update_field_test0(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "f": string
// CHECK-NEXT:  %4 = AsNumericInst (:number|bigint) %3: any
// CHECK-NEXT:  %5 = UnaryIncInst (:any) %4: number|bigint
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: any, %2: any, "f": string
// CHECK-NEXT:  %7 = ReturnInst %4: number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test1(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "f": string
// CHECK-NEXT:  %4 = AsNumericInst (:number|bigint) %3: any
// CHECK-NEXT:  %5 = UnaryDecInst (:any) %4: number|bigint
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: any, %2: any, "f": string
// CHECK-NEXT:  %7 = ReturnInst %4: number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test2(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "f": string
// CHECK-NEXT:  %4 = UnaryIncInst (:any) %3: any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: any, %2: any, "f": string
// CHECK-NEXT:  %6 = ReturnInst %4: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test3(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "f": string
// CHECK-NEXT:  %4 = UnaryDecInst (:any) %3: any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: any, %2: any, "f": string
// CHECK-NEXT:  %6 = ReturnInst %4: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test0(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = AsNumericInst (:number|bigint) %2: any
// CHECK-NEXT:  %4 = UnaryIncInst (:any) %3: number|bigint
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = ReturnInst %3: number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test1(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = AsNumericInst (:number|bigint) %2: any
// CHECK-NEXT:  %4 = UnaryDecInst (:any) %3: number|bigint
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = ReturnInst %3: number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test2(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = UnaryIncInst (:any) %2: any
// CHECK-NEXT:  %4 = StoreFrameInst %3: any, [x]: any
// CHECK-NEXT:  %5 = ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test3(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = UnaryDecInst (:any) %2: any
// CHECK-NEXT:  %4 = StoreFrameInst %3: any, [x]: any
// CHECK-NEXT:  %5 = ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

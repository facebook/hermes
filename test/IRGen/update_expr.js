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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "update_field_test0" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "update_field_test1" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "update_field_test2" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "update_field_test3" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "update_variable_test0" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "update_variable_test1" : string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "update_variable_test2" : string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "update_variable_test3" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %update_field_test0()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "update_field_test0" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %update_field_test1()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "update_field_test1" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %update_field_test2()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "update_field_test2" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %update_field_test3()
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "update_field_test3" : string
// CHECK-NEXT:  %16 = CreateFunctionInst %update_variable_test0()
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16 : closure, globalObject : object, "update_variable_test0" : string
// CHECK-NEXT:  %18 = CreateFunctionInst %update_variable_test1()
// CHECK-NEXT:  %19 = StorePropertyLooseInst %18 : closure, globalObject : object, "update_variable_test1" : string
// CHECK-NEXT:  %20 = CreateFunctionInst %update_variable_test2()
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20 : closure, globalObject : object, "update_variable_test2" : string
// CHECK-NEXT:  %22 = CreateFunctionInst %update_variable_test3()
// CHECK-NEXT:  %23 = StorePropertyLooseInst %22 : closure, globalObject : object, "update_variable_test3" : string
// CHECK-NEXT:  %24 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %25 = StoreStackInst undefined : undefined, %24
// CHECK-NEXT:  %26 = LoadStackInst %24
// CHECK-NEXT:  %27 = ReturnInst %26
// CHECK-NEXT:function_end

// CHECK:function update_field_test0(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %o
// CHECK-NEXT:  %1 = StoreFrameInst %0, [o]
// CHECK-NEXT:  %2 = LoadFrameInst [o]
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = AsNumericInst %3
// CHECK-NEXT:  %5 = UnaryOperatorInst '++', %4 : number|bigint
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5, %2, "f" : string
// CHECK-NEXT:  %7 = ReturnInst %4 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test1(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %o
// CHECK-NEXT:  %1 = StoreFrameInst %0, [o]
// CHECK-NEXT:  %2 = LoadFrameInst [o]
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = AsNumericInst %3
// CHECK-NEXT:  %5 = UnaryOperatorInst '--', %4 : number|bigint
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5, %2, "f" : string
// CHECK-NEXT:  %7 = ReturnInst %4 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test2(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %o
// CHECK-NEXT:  %1 = StoreFrameInst %0, [o]
// CHECK-NEXT:  %2 = LoadFrameInst [o]
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4, %2, "f" : string
// CHECK-NEXT:  %6 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test3(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %o
// CHECK-NEXT:  %1 = StoreFrameInst %0, [o]
// CHECK-NEXT:  %2 = LoadFrameInst [o]
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = UnaryOperatorInst '--', %3
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4, %2, "f" : string
// CHECK-NEXT:  %6 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test0(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = AsNumericInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3 : number|bigint
// CHECK-NEXT:  %5 = StoreFrameInst %4, [x]
// CHECK-NEXT:  %6 = ReturnInst %3 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test1(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = AsNumericInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '--', %3 : number|bigint
// CHECK-NEXT:  %5 = StoreFrameInst %4, [x]
// CHECK-NEXT:  %6 = ReturnInst %3 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test2(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = UnaryOperatorInst '++', %2
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x]
// CHECK-NEXT:  %5 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test3(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = UnaryOperatorInst '--', %2
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x]
// CHECK-NEXT:  %5 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

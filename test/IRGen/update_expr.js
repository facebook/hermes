/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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
// CHECK-NEXT:frame = [], globals = [update_field_test0, update_field_test1, update_field_test2, update_field_test3, update_variable_test0, update_variable_test1, update_variable_test2, update_variable_test3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %update_field_test0()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "update_field_test0" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %update_field_test1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "update_field_test1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %update_field_test2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "update_field_test2" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %update_field_test3()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "update_field_test3" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %update_variable_test0()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "update_variable_test0" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %update_variable_test1()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "update_variable_test1" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %update_variable_test2()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "update_variable_test2" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %update_variable_test3()
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "update_variable_test3" : string
// CHECK-NEXT:  %16 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %17 = StoreStackInst undefined : undefined, %16
// CHECK-NEXT:  %18 = LoadStackInst %16
// CHECK-NEXT:  %19 = ReturnInst %18
// CHECK-NEXT:function_end

// CHECK:function update_field_test0(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %o, [o]
// CHECK-NEXT:  %1 = LoadFrameInst [o]
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "f" : string
// CHECK-NEXT:  %3 = AsNumericInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3 : number|bigint
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4, %1, "f" : string
// CHECK-NEXT:  %6 = ReturnInst %3 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test1(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %o, [o]
// CHECK-NEXT:  %1 = LoadFrameInst [o]
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "f" : string
// CHECK-NEXT:  %3 = AsNumericInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '--', %3 : number|bigint
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4, %1, "f" : string
// CHECK-NEXT:  %6 = ReturnInst %3 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test2(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %o, [o]
// CHECK-NEXT:  %1 = LoadFrameInst [o]
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "f" : string
// CHECK-NEXT:  %3 = UnaryOperatorInst '++', %2
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3, %1, "f" : string
// CHECK-NEXT:  %5 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test3(o)
// CHECK-NEXT:frame = [o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %o, [o]
// CHECK-NEXT:  %1 = LoadFrameInst [o]
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "f" : string
// CHECK-NEXT:  %3 = UnaryOperatorInst '--', %2
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3, %1, "f" : string
// CHECK-NEXT:  %5 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test0(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = LoadFrameInst [x]
// CHECK-NEXT:  %2 = AsNumericInst %1
// CHECK-NEXT:  %3 = UnaryOperatorInst '++', %2 : number|bigint
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x]
// CHECK-NEXT:  %5 = ReturnInst %2 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test1(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = LoadFrameInst [x]
// CHECK-NEXT:  %2 = AsNumericInst %1
// CHECK-NEXT:  %3 = UnaryOperatorInst '--', %2 : number|bigint
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x]
// CHECK-NEXT:  %5 = ReturnInst %2 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test2(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = LoadFrameInst [x]
// CHECK-NEXT:  %2 = UnaryOperatorInst '++', %1
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test3(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = LoadFrameInst [x]
// CHECK-NEXT:  %2 = UnaryOperatorInst '--', %1
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [update_field_test0, update_field_test1, update_field_test2, update_field_test3, update_variable_test0, update_variable_test1, update_variable_test2, update_variable_test3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %update_field_test0#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "update_field_test0" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %update_field_test1#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "update_field_test1" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %update_field_test2#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "update_field_test2" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %update_field_test3#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "update_field_test3" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %update_variable_test0#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "update_variable_test0" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %update_variable_test1#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "update_variable_test1" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %update_variable_test2#0#1()#8, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "update_variable_test2" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %update_variable_test3#0#1()#9, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "update_variable_test3" : string
// CHECK-NEXT:  %17 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %18 = StoreStackInst undefined : undefined, %17
// CHECK-NEXT:  %19 = LoadStackInst %17
// CHECK-NEXT:  %20 = ReturnInst %19
// CHECK-NEXT:function_end

// CHECK:function update_field_test0#0#1(o)#2
// CHECK-NEXT:frame = [o#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_field_test0#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %o, [o#2], %0
// CHECK-NEXT:  %2 = LoadFrameInst [o#2], %0
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = AsNumericInst %3
// CHECK-NEXT:  %5 = UnaryOperatorInst '++', %4 : number|bigint
// CHECK-NEXT:  %6 = StorePropertyInst %5, %2, "f" : string
// CHECK-NEXT:  %7 = ReturnInst %4 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test1#0#1(o)#3
// CHECK-NEXT:frame = [o#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_field_test1#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %o, [o#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [o#3], %0
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = AsNumericInst %3
// CHECK-NEXT:  %5 = UnaryOperatorInst '--', %4 : number|bigint
// CHECK-NEXT:  %6 = StorePropertyInst %5, %2, "f" : string
// CHECK-NEXT:  %7 = ReturnInst %4 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test2#0#1(o)#4
// CHECK-NEXT:frame = [o#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_field_test2#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %o, [o#4], %0
// CHECK-NEXT:  %2 = LoadFrameInst [o#4], %0
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3
// CHECK-NEXT:  %5 = StorePropertyInst %4, %2, "f" : string
// CHECK-NEXT:  %6 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_field_test3#0#1(o)#5
// CHECK-NEXT:frame = [o#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_field_test3#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %o, [o#5], %0
// CHECK-NEXT:  %2 = LoadFrameInst [o#5], %0
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "f" : string
// CHECK-NEXT:  %4 = UnaryOperatorInst '--', %3
// CHECK-NEXT:  %5 = StorePropertyInst %4, %2, "f" : string
// CHECK-NEXT:  %6 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test0#0#1(x)#6
// CHECK-NEXT:frame = [x#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_variable_test0#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#6], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#6], %0
// CHECK-NEXT:  %3 = AsNumericInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '++', %3 : number|bigint
// CHECK-NEXT:  %5 = StoreFrameInst %4, [x#6], %0
// CHECK-NEXT:  %6 = ReturnInst %3 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test1#0#1(x)#7
// CHECK-NEXT:frame = [x#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_variable_test1#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#7], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#7], %0
// CHECK-NEXT:  %3 = AsNumericInst %2
// CHECK-NEXT:  %4 = UnaryOperatorInst '--', %3 : number|bigint
// CHECK-NEXT:  %5 = StoreFrameInst %4, [x#7], %0
// CHECK-NEXT:  %6 = ReturnInst %3 : number|bigint
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test2#0#1(x)#8
// CHECK-NEXT:frame = [x#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_variable_test2#0#1()#8}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#8], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#8], %0
// CHECK-NEXT:  %3 = UnaryOperatorInst '++', %2
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x#8], %0
// CHECK-NEXT:  %5 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function update_variable_test3#0#1(x)#9
// CHECK-NEXT:frame = [x#9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{update_variable_test3#0#1()#9}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#9], %0
// CHECK-NEXT:  %2 = LoadFrameInst [x#9], %0
// CHECK-NEXT:  %3 = UnaryOperatorInst '--', %2
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x#9], %0
// CHECK-NEXT:  %5 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

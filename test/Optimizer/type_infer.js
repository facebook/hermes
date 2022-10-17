/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// Make sure we can remove all trampolines from our code.

function sink() {}

function test_one(x,y) {
  var sink = x;

  sink(x + 2);

  sink(2 + 2);

  sink(x * 2 + x * 2);

  sink((x|0) + (x|0));

  sink("hi" + "bye");

  sink(x + y);

  sink("hi" + y);

  sink(null + null);

  sink({} + {});

  sink(undefined + undefined);
}

function test_unary(x) {
  var sk = sink;
  var y = x;

  sk(void x); // Undef
  sk(!x);     // bool
  sk(y++);    // number
  sk(typeof(x)); // string

}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [sink, test_one, test_unary]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %sink#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test_one#0#1()#3 : undefined, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_unary#0#1()#4 : undefined, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_unary" : string
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink#0#1()#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_one#0#1(x, y)#3 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_one#0#1()#3}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %x, 2 : number
// CHECK-NEXT:  %2 = CallInst %x, undefined : undefined, %1 : string|number
// CHECK-NEXT:  %3 = CallInst %x, undefined : undefined, 4 : number
// CHECK-NEXT:  %4 = BinaryOperatorInst '*', %x, 2 : number
// CHECK-NEXT:  %5 = BinaryOperatorInst '*', %x, 2 : number
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4 : number, %5 : number
// CHECK-NEXT:  %7 = CallInst %x, undefined : undefined, %6 : number
// CHECK-NEXT:  %8 = AsInt32Inst %x
// CHECK-NEXT:  %9 = AsInt32Inst %x
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %8 : number, %9 : number
// CHECK-NEXT:  %11 = CallInst %x, undefined : undefined, %10 : number
// CHECK-NEXT:  %12 = CallInst %x, undefined : undefined, "hibye" : string
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %14 = CallInst %x, undefined : undefined, %13 : string|number|bigint
// CHECK-NEXT:  %15 = BinaryOperatorInst '+', "hi" : string, %y
// CHECK-NEXT:  %16 = CallInst %x, undefined : undefined, %15 : string
// CHECK-NEXT:  %17 = CallInst %x, undefined : undefined, 0 : number
// CHECK-NEXT:  %18 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %19 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %20 = BinaryOperatorInst '+', %18 : object, %19 : object
// CHECK-NEXT:  %21 = CallInst %x, undefined : undefined, %20 : string|number
// CHECK-NEXT:  %22 = CallInst %x, undefined : undefined, NaN : number
// CHECK-NEXT:  %23 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_unary#0#1(x)#4 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_unary#0#1()#4}
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = UnaryOperatorInst 'void', %x
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, %2 : undefined
// CHECK-NEXT:  %4 = UnaryOperatorInst '!', %x
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, %4 : boolean
// CHECK-NEXT:  %6 = AsNumericInst %x
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, %6 : number|bigint
// CHECK-NEXT:  %8 = UnaryOperatorInst 'typeof', %x
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, %8 : string
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

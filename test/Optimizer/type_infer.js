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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [sink, test_one, test_unary]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %sink() : undefined
// CHECK-NEXT:  %1 = StorePropertyInst %0 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %test_one() : undefined
// CHECK-NEXT:  %3 = StorePropertyInst %2 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test_unary() : undefined
// CHECK-NEXT:  %5 = StorePropertyInst %4 : closure, globalObject : object, "test_unary" : string
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_one(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '+', %x, 2 : number
// CHECK-NEXT:  %1 = CallInst %x, undefined : undefined, %0 : string|number
// CHECK-NEXT:  %2 = CallInst %x, undefined : undefined, 4 : number
// CHECK-NEXT:  %3 = BinaryOperatorInst '*', %x, 2 : number
// CHECK-NEXT:  %4 = BinaryOperatorInst '*', %x, 2 : number
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3 : number, %4 : number
// CHECK-NEXT:  %6 = CallInst %x, undefined : undefined, %5 : number
// CHECK-NEXT:  %7 = AsInt32Inst %x
// CHECK-NEXT:  %8 = AsInt32Inst %x
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %7 : number, %8 : number
// CHECK-NEXT:  %10 = CallInst %x, undefined : undefined, %9 : number
// CHECK-NEXT:  %11 = CallInst %x, undefined : undefined, "hibye" : string
// CHECK-NEXT:  %12 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %13 = CallInst %x, undefined : undefined, %12 : string|number|bigint
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', "hi" : string, %y
// CHECK-NEXT:  %15 = CallInst %x, undefined : undefined, %14 : string
// CHECK-NEXT:  %16 = CallInst %x, undefined : undefined, 0 : number
// CHECK-NEXT:  %17 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %18 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %19 = BinaryOperatorInst '+', %17 : object, %18 : object
// CHECK-NEXT:  %20 = CallInst %x, undefined : undefined, %19 : string|number
// CHECK-NEXT:  %21 = CallInst %x, undefined : undefined, NaN : number
// CHECK-NEXT:  %22 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_unary(x) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %1 = UnaryOperatorInst 'void', %x
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, %1 : undefined
// CHECK-NEXT:  %3 = UnaryOperatorInst '!', %x
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, %3 : boolean
// CHECK-NEXT:  %5 = AsNumericInst %x
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, %5 : number|bigint
// CHECK-NEXT:  %7 = UnaryOperatorInst 'typeof', %x
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, %7 : string
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

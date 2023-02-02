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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "sink" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_one" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_unary" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %sink() : undefined
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_one() : undefined
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_unary() : undefined
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "test_unary" : string
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_one(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0, 2 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, %2 : string|number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 4 : number
// CHECK-NEXT:  %5 = BinaryOperatorInst '*', %0, 2 : number
// CHECK-NEXT:  %6 = BinaryOperatorInst '*', %0, 2 : number
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5 : number, %6 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, %7 : number
// CHECK-NEXT:  %9 = AsInt32Inst %0
// CHECK-NEXT:  %10 = AsInt32Inst %0
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %9 : number, %10 : number
// CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, %11 : number
// CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, "hibye" : string
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', %0, %1
// CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, %14 : string|number|bigint
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', "hi" : string, %1
// CHECK-NEXT:  %17 = CallInst %0, undefined : undefined, %16 : string
// CHECK-NEXT:  %18 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %19 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %20 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %21 = BinaryOperatorInst '+', %19 : object, %20 : object
// CHECK-NEXT:  %22 = CallInst %0, undefined : undefined, %21 : string|number
// CHECK-NEXT:  %23 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %24 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_unary(x) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = UnaryOperatorInst 'void', %0
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, %2 : undefined
// CHECK-NEXT:  %4 = UnaryOperatorInst '!', %0
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, %4 : boolean
// CHECK-NEXT:  %6 = AsNumericInst %0
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, %6 : number|bigint
// CHECK-NEXT:  %8 = UnaryOperatorInst 'typeof', %0
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, %8 : string
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O | %FileCheck %s --match-full-lines

// Make sure we can remove all trampolines from our code.

function sink() {}

//CHECK-LABEL:function test_one(x, y) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:

function test_one(x,y) {
  var sink = x;

//CHECK-NEXT:  %0 = BinaryOperatorInst '+', %x, 2 : number
//CHECK-NEXT:  %1 = CallInst %x, undefined : undefined, %0 : string|number
  sink(x + 2);

//CHECK-NEXT:  %2 = CallInst %x, undefined : undefined, 4 : number
  sink(2 + 2);

//CHECK-NEXT:  %3 = BinaryOperatorInst '*', %x, 2 : number
//CHECK-NEXT:  %4 = BinaryOperatorInst '*', %x, 2 : number
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3 : number, %4 : number
//CHECK-NEXT:  %6 = CallInst %x, undefined : undefined, %5 : number
  sink(x * 2 + x * 2);

//CHECK-NEXT:  %7 = AsInt32Inst %x
//CHECK-NEXT:  %8 = AsInt32Inst %x
//CHECK-NEXT:  %9 = BinaryOperatorInst '+', %7 : int32, %8 : int32
//CHECK-NEXT:  %10 = CallInst %x, undefined : undefined, %9 : number
  sink((x|0) + (x|0));

//CHECK-NEXT:  %11 = CallInst %x, undefined : undefined, "hibye" : string
  sink("hi" + "bye");

//CHECK-NEXT:  %12 = BinaryOperatorInst '+', %x, %y
//CHECK-NEXT:  %13 = CallInst %x, undefined : undefined, %12 : string|number
  sink(x + y);

//CHECK-NEXT:  %14 = BinaryOperatorInst '+', "hi" : string, %y
//CHECK-NEXT:  %15 = CallInst %x, undefined : undefined, %14 : string
  sink("hi" + y);

//CHECK-NEXT:  %16 = CallInst %x, undefined : undefined, 0 : number
  sink(null + null);

//CHECK-NEXT:  %17 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  %18 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  %19 = BinaryOperatorInst '+', %17 : object, %18 : object
//CHECK-NEXT:  %20 = CallInst %x, undefined : undefined, %19 : string|number
  sink({} + {});

//CHECK-NEXT:  %21 = CallInst %x, undefined : undefined, NaN : number
  sink(undefined + undefined);
//CHECK-NEXT:  %22 = ReturnInst undefined : undefined
}

//CHECK-LABEL:function test_unary(x) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = LoadPropertyInst globalObject : object, "sink" : string
//CHECK-NEXT:  %1 = UnaryOperatorInst 'void', %x
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, %1 : undefined
//CHECK-NEXT:  %3 = UnaryOperatorInst '!', %x
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, %3 : boolean
//CHECK-NEXT:  %5 = AsNumberInst %x
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, %5 : number
//CHECK-NEXT:  %7 = UnaryOperatorInst 'typeof', %x
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, %7 : string
//CHECK-NEXT:  %9 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_unary(x) {
  var sk = sink;
  var y = x;

  sk(void x); // Undef
  sk(!x);     // bool
  sk(y++);    // number
  sk(typeof(x)); // string

}

//CHECK-LABEL:function test_unsigned(x, y) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '>>>', %x, 0 : number
//CHECK-NEXT:  %1 = BinaryOperatorInst '>>>', %y, 0 : number
//CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0 : uint32, %1 : uint32
//CHECK-NEXT:  %3 = StorePropertyInst %2 : number, globalObject : object, "z" : string
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_unsigned(x, y) {
  x = x >>> 0; // Unsigned
  y = y >>> 0; // Unsigned
  z = x + y; // Number
}

//CHECK-LABEL:function test_bitwise_not(x) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = UnaryOperatorInst '~', %x
//CHECK-NEXT:  %1 = BinaryOperatorInst '*', %0 : int32, 2 : number
//CHECK-NEXT:  %2 = StorePropertyInst %1 : number, globalObject : object, "y" : string
//CHECK-NEXT:  %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_bitwise_not(x) {
  x = ~x; // Signed
  y = x * 2; // Number
}

//CHECK-LABEL:function test_shift(x, y) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '<<', %x, 1 : number
//CHECK-NEXT:  %1 = BinaryOperatorInst '>>', %y, 3 : number
//CHECK-NEXT:  %2 = BinaryOperatorInst '/', %0 : int32, %1 : int32
//CHECK-NEXT:  %3 = StorePropertyInst %2 : number, globalObject : object, "z" : string
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_shift(x, y) {
  x = x << 1; // Signed
  y = y >> 3; // Signed
  z = x / y; // Number
}

//CHECK-LABEL:function test_and_xor(x, y) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '&', %x, 10678 : number
//CHECK-NEXT:  %1 = BinaryOperatorInst '^', %y, 10086 : number
//CHECK-NEXT:  %2 = BinaryOperatorInst '-', %0 : int32, %1 : int32
//CHECK-NEXT:  %3 = StorePropertyInst %2 : number, globalObject : object, "z" : string
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_and_xor(x, y) {
  x = x & 10678; // Signed
  y = y ^ 10086; // Signed
  z = x - y; // Number
}

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

function foo() {
  return 0;
}

function bar(a,b,c,d,e,f,g,h) {
  b += a;
  c += b;
  d += c;
  e += d;
  f += e;
  g += f;
  h += a;
  foo(h, g, f, e, d, c, b, a);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [foo, bar]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...5) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg2 @1 [2...4) 	%1 = HBCCreateFunctionInst %foo#0#1()#2 : number, %0
// CHECK-NEXT:  $Reg1 @2 [3...6) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "foo" : string
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = HBCCreateFunctionInst %bar#0#1()#3 : undefined, %0
// CHECK-NEXT:  $Reg0 @5 [empty]	%5 = StorePropertyInst %4 : closure, %2 : object, "bar" : string
// CHECK-NEXT:  $Reg0 @6 [7...8) 	%6 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @7 [empty]	%7 = ReturnInst %6 : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#2 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg0 @1 [empty]	%1 = ReturnInst %0 : number
// CHECK-NEXT:function_end

// CHECK:function bar#0#1(a, b, c, d, e, f, g, h)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg9 @0 [1...19) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @1 [2...3) 	%1 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg8 @2 [3...19) 	%2 = BinaryOperatorInst '+', %1, %0
// CHECK-NEXT:  $Reg0 @3 [4...5) 	%3 = HBCLoadParamInst 3 : number
// CHECK-NEXT:  $Reg7 @4 [5...19) 	%4 = BinaryOperatorInst '+', %3, %2 : string|number|bigint
// CHECK-NEXT:  $Reg0 @5 [6...7) 	%5 = HBCLoadParamInst 4 : number
// CHECK-NEXT:  $Reg6 @6 [7...19) 	%6 = BinaryOperatorInst '+', %5, %4 : string|number|bigint
// CHECK-NEXT:  $Reg0 @7 [8...9) 	%7 = HBCLoadParamInst 5 : number
// CHECK-NEXT:  $Reg5 @8 [9...19) 	%8 = BinaryOperatorInst '+', %7, %6 : string|number|bigint
// CHECK-NEXT:  $Reg0 @9 [10...11) 	%9 = HBCLoadParamInst 6 : number
// CHECK-NEXT:  $Reg4 @10 [11...19) 	%10 = BinaryOperatorInst '+', %9, %8 : string|number|bigint
// CHECK-NEXT:  $Reg0 @11 [12...13) 	%11 = HBCLoadParamInst 7 : number
// CHECK-NEXT:  $Reg3 @12 [13...19) 	%12 = BinaryOperatorInst '+', %11, %10 : string|number|bigint
// CHECK-NEXT:  $Reg0 @13 [14...15) 	%13 = HBCLoadParamInst 8 : number
// CHECK-NEXT:  $Reg2 @14 [15...19) 	%14 = BinaryOperatorInst '+', %13, %0
// CHECK-NEXT:  $Reg0 @15 [16...17) 	%15 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg1 @16 [17...19) 	%16 = LoadPropertyInst %15 : object, "foo" : string
// CHECK-NEXT:  $Reg0 @17 [18...20) 	%17 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg1 @18 [empty]	%18 = CallInst %16, %17 : undefined, %14 : string|number|bigint, %12 : string|number|bigint, %10 : string|number|bigint, %8 : string|number|bigint, %6 : string|number|bigint, %4 : string|number|bigint, %2 : string|number|bigint, %0
// CHECK-NEXT:  $Reg0 @19 [empty]	%19 = ReturnInst %17 : undefined
// CHECK-NEXT:function_end

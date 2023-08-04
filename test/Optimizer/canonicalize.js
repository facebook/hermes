/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -Og | %FileCheckOrRegen %s

!0;
!1;
void 0;
void 1;
""+0;
0+"";
0|"";
"1"|0;
+"";
+"0";

function looseInequalNullToUndefined(a) {
  return a != null;
}

function looseEqualNullToUndefined(a) {
  return a == null;
}

function nullCoalesceUsesUndefined(a) {
  return a ?? undefined;
}

// CHECK:function global#0()#1
// CHECK-NEXT:globals = [looseInequalNullToUndefined, looseEqualNullToUndefined, nullCoalesceUsesUndefined]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %looseInequalNullToUndefined#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "looseInequalNullToUndefined" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %looseEqualNullToUndefined#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "looseEqualNullToUndefined" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %nullCoalesceUsesUndefined#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "nullCoalesceUsesUndefined" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = StoreStackInst true : boolean, %7
// CHECK-NEXT:  %10 = StoreStackInst false : boolean, %7
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', "" : string, 0 : number
// CHECK-NEXT:  %14 = StoreStackInst %13, %7
// CHECK-NEXT:  %15 = AddEmptyStringInst 0 : number
// CHECK-NEXT:  %16 = StoreStackInst %15 : string, %7
// CHECK-NEXT:  %17 = BinaryOperatorInst '|', 0 : number, "" : string
// CHECK-NEXT:  %18 = StoreStackInst %17, %7
// CHECK-NEXT:  %19 = AsInt32Inst "1" : string
// CHECK-NEXT:  %20 = StoreStackInst %19 : number, %7
// CHECK-NEXT:  %21 = AsNumberInst "" : string
// CHECK-NEXT:  %22 = StoreStackInst %21 : number, %7
// CHECK-NEXT:  %23 = AsNumberInst "0" : string
// CHECK-NEXT:  %24 = StoreStackInst %23 : number, %
// CHECK-NEXT:  %25 = LoadStackInst %7
// CHECK-NEXT:  %26 = ReturnInst %25
// CHECK-NEXT:function_end

// CHECK:function looseInequalNullToUndefined#0#1(a)#2
// CHECK-NEXT:S{looseInequalNullToUndefined#0#1()#2} = [a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{looseInequalNullToUndefined#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %3 = BinaryOperatorInst '!=', %2, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function looseEqualNullToUndefined#0#1(a)#3
// CHECK-NEXT:S{looseEqualNullToUndefined#0#1()#3} = [a#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{looseEqualNullToUndefined#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#3], %0
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function nullCoalesceUsesUndefined#0#1(a)#4
// CHECK-NEXT:S{nullCoalesceUsesUndefined#0#1()#4} = [a#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{nullCoalesceUsesUndefined#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#4], %0
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %3 = LoadFrameInst [a#4], %0
// CHECK-NEXT:  %4 = StoreStackInst %3, %2
// CHECK-NEXT:  %5 = BinaryOperatorInst '==', %3, undefined : undefined
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %8 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadStackInst %2
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
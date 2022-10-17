/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function dummy() {
  return;
}
function emptyString() {
  return dummy``;
}

function oneString() {
  return dummy`hello`;
}

function oneSub() {
  return dummy`${666}`;
}

function dup() {
  return dummy`hello world${1 + 2}!!!`;
}

function notDup() {
  return dummy`hello\nworld${1 + 2}!!!`;
}

function memberExpr() {
  var obj = {func: dummy};
  return obj.func`hello world!`;
}

function callExpr() {
  function func() {
    return function() {
      return;
    }
  }
  return func()`hello world!`;
}

/// Some more test cases to check template object id.

function dup2() {
  return dummy`hello world${1 + 2}!!!`;
}

function dup3() {
  return dummy`hello world${7}!!!`;
}

function helloWorld() {
  return dummy`hello${0} world!!!`;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [dummy, emptyString, oneString, oneSub, dup, notDup, memberExpr, callExpr, dup2, dup3, helloWorld]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %dummy#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %emptyString#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "emptyString" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %oneString#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "oneString" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %oneSub#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "oneSub" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %dup#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "dup" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %notDup#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "notDup" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %memberExpr#0#1()#8, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "memberExpr" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %callExpr#0#1()#9, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "callExpr" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %dup2#0#1()#12, %0
// CHECK-NEXT:  %18 = StorePropertyInst %17 : closure, globalObject : object, "dup2" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %dup3#0#1()#13, %0
// CHECK-NEXT:  %20 = StorePropertyInst %19 : closure, globalObject : object, "dup3" : string
// CHECK-NEXT:  %21 = CreateFunctionInst %helloWorld#0#1()#14, %0
// CHECK-NEXT:  %22 = StorePropertyInst %21 : closure, globalObject : object, "helloWorld" : string
// CHECK-NEXT:  %23 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %24 = StoreStackInst undefined : undefined, %23
// CHECK-NEXT:  %25 = LoadStackInst %23
// CHECK-NEXT:  %26 = ReturnInst %25
// CHECK-NEXT:function_end

// CHECK:function dummy#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{dummy#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function emptyString#0#1()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{emptyString#0#1()#3}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 0 : number, true : boolean, "" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function oneString#0#1()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{oneString#0#1()#4}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 1 : number, true : boolean, "hello" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function oneSub#0#1()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{oneSub#0#1()#5}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 2 : number, true : boolean, "" : string, "" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %1, 666 : number
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function dup#0#1()#6
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{dup#0#1()#6}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined, %1, %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function notDup#0#1()#7
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{notDup#0#1()#7}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 4 : number, false : boolean, "hello\\\\nworld" : string, "!!!" : string, "hello\\nworld" : string, "!!!" : string
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined, %1, %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function memberExpr#0#1()#8
// CHECK-NEXT:frame = [obj#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{memberExpr#0#1()#8}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [obj#8], %0
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = AllocObjectLiteralInst "func" : string, %2
// CHECK-NEXT:  %4 = StoreFrameInst %3 : object, [obj#8], %0
// CHECK-NEXT:  %5 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 5 : number, true : boolean, "hello world!" : string
// CHECK-NEXT:  %6 = LoadFrameInst [obj#8], %0
// CHECK-NEXT:  %7 = LoadPropertyInst %6, "func" : string
// CHECK-NEXT:  %8 = CallInst %7, %6, %5
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function callExpr#0#1()#9
// CHECK-NEXT:frame = [func#9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{callExpr#0#1()#9}
// CHECK-NEXT:  %1 = CreateFunctionInst %func#1#9()#10, %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [func#9], %0
// CHECK-NEXT:  %3 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 5 : number, true : boolean, "hello world!" : string
// CHECK-NEXT:  %4 = LoadFrameInst [func#9], %0
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined, %3
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function func#1#9()#10
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{func#1#9()#10}
// CHECK-NEXT:  %1 = CreateFunctionInst %""#9#10()#11, %0
// CHECK-NEXT:  %2 = ReturnInst %1 : closure
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function ""#9#10()#11
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{""#9#10()#11}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function dup2#0#1()#12
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{dup2#0#1()#12}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined, %1, %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function dup3#0#1()#13
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{dup3#0#1()#13}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %1, 7 : number
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function helloWorld#0#1()#14
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{helloWorld#0#1()#14}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 6 : number, true : boolean, "hello" : string, " world!!!" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %1, 0 : number
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

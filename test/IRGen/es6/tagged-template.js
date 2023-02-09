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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "dummy" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "emptyString" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "oneString" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "oneSub" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "dup" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "notDup" : string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "memberExpr" : string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "callExpr" : string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "dup2" : string
// CHECK-NEXT:  %9 = DeclareGlobalVarInst "dup3" : string
// CHECK-NEXT:  %10 = DeclareGlobalVarInst "helloWorld" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %dummy()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "dummy" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %emptyString()
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13 : closure, globalObject : object, "emptyString" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %oneString()
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15 : closure, globalObject : object, "oneString" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %oneSub()
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17 : closure, globalObject : object, "oneSub" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %dup()
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19 : closure, globalObject : object, "dup" : string
// CHECK-NEXT:  %21 = CreateFunctionInst %notDup()
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21 : closure, globalObject : object, "notDup" : string
// CHECK-NEXT:  %23 = CreateFunctionInst %memberExpr()
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23 : closure, globalObject : object, "memberExpr" : string
// CHECK-NEXT:  %25 = CreateFunctionInst %callExpr()
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25 : closure, globalObject : object, "callExpr" : string
// CHECK-NEXT:  %27 = CreateFunctionInst %dup2()
// CHECK-NEXT:  %28 = StorePropertyLooseInst %27 : closure, globalObject : object, "dup2" : string
// CHECK-NEXT:  %29 = CreateFunctionInst %dup3()
// CHECK-NEXT:  %30 = StorePropertyLooseInst %29 : closure, globalObject : object, "dup3" : string
// CHECK-NEXT:  %31 = CreateFunctionInst %helloWorld()
// CHECK-NEXT:  %32 = StorePropertyLooseInst %31 : closure, globalObject : object, "helloWorld" : string
// CHECK-NEXT:  %33 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %34 = StoreStackInst undefined : undefined, %33
// CHECK-NEXT:  %35 = LoadStackInst %33
// CHECK-NEXT:  %36 = ReturnInst %35
// CHECK-NEXT:function_end

// CHECK:function dummy()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function emptyString()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 0 : number, true : boolean, "" : string
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %2 = CallInst %1, empty, empty, undefined : undefined, %0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function oneString()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 1 : number, true : boolean, "hello" : string
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %2 = CallInst %1, empty, empty, undefined : undefined, %0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function oneSub()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 2 : number, true : boolean, "" : string, "" : string
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %2 = CallInst %1, empty, empty, undefined : undefined, %0, 666 : number
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function dup()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 3 : number, true : boolean, "hello world" : string, "!!!" : string
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, empty, empty, undefined : undefined, %0, %1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function notDup()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 4 : number, false : boolean, "hello\\\\nworld" : string, "!!!" : string, "hello\\nworld" : string, "!!!" : string
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, empty, empty, undefined : undefined, %0, %1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function memberExpr()
// CHECK-NEXT:frame = [obj]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [obj]
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %2 = AllocObjectLiteralInst "func" : string, %1
// CHECK-NEXT:  %3 = StoreFrameInst %2 : object, [obj]
// CHECK-NEXT:  %4 = GetTemplateObjectInst 5 : number, true : boolean, "hello world!" : string
// CHECK-NEXT:  %5 = LoadFrameInst [obj]
// CHECK-NEXT:  %6 = LoadPropertyInst %5, "func" : string
// CHECK-NEXT:  %7 = CallInst %6, empty, empty, %5, %4
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function callExpr()
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %func()
// CHECK-NEXT:  %1 = StoreFrameInst %0 : closure, [func]
// CHECK-NEXT:  %2 = GetTemplateObjectInst 5 : number, true : boolean, "hello world!" : string
// CHECK-NEXT:  %3 = LoadFrameInst [func]
// CHECK-NEXT:  %4 = CallInst %3, empty, empty, undefined : undefined
// CHECK-NEXT:  %5 = CallInst %4, empty, empty, undefined : undefined, %2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function dup2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 3 : number, true : boolean, "hello world" : string, "!!!" : string
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %3 = CallInst %2, empty, empty, undefined : undefined, %0, %1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function dup3()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 3 : number, true : boolean, "hello world" : string, "!!!" : string
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %2 = CallInst %1, empty, empty, undefined : undefined, %0, 7 : number
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function helloWorld()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst 6 : number, true : boolean, "hello" : string, " world!!!" : string
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
// CHECK-NEXT:  %2 = CallInst %1, empty, empty, undefined : undefined, %0, 0 : number
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function func()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %""()
// CHECK-NEXT:  %1 = ReturnInst %0 : closure
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function ""()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

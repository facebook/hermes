/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

function dummy() {
  return;
}
function emptyString() {
  return dummy``;
}
//CHECK-LABEL:function emptyString()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 0 : number, true : boolean, "" : string
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, %0
//CHECK-NEXT:  %3 = ReturnInst %2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function oneString() {
  return dummy`hello`;
}
//CHECK-LABEL:function oneString()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 1 : number, true : boolean, "hello" : string
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, %0
//CHECK-NEXT:  %3 = ReturnInst %2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function oneSub() {
  return dummy`${666}`;
}
//CHECK-LABEL:function oneSub()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 2 : number, true : boolean, "" : string, "" : string
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, %0, 666 : number
//CHECK-NEXT:  %3 = ReturnInst %2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function dup() {
  return dummy`hello world${1 + 2}!!!`;
}
//CHECK-LABEL:function dup()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
//CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1 : number, 2 : number
//CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %0, %1
//CHECK-NEXT:  %4 = ReturnInst %3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function notDup() {
  return dummy`hello\nworld${1 + 2}!!!`;
}
//CHECK-LABEL:function notDup()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 4 : number, false : boolean, "hello\\\\nworld" : string, "!!!" : string, "hello\\nworld" : string, "!!!" : string
//CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1 : number, 2 : number
//CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %0, %1
//CHECK-NEXT:  %4 = ReturnInst %3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end


function memberExpr() {
  var obj = {func: dummy};
  return obj.func`hello world!`;
}
//CHECK-LABEL:function memberExpr()
//CHECK-NEXT:frame = [obj]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [obj]
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %2 = AllocObjectLiteralInst "func" : string, %1
//CHECK-NEXT:  %3 = StoreFrameInst %2 : object, [obj]
//CHECK-NEXT:  %4 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 5 : number, true : boolean, "hello world!" : string
//CHECK-NEXT:  %5 = LoadFrameInst [obj]
//CHECK-NEXT:  %6 = LoadPropertyInst %5, "func" : string
//CHECK-NEXT:  %7 = CallInst %6, %5, %4
//CHECK-NEXT:  %8 = ReturnInst %7
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %9 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function callExpr() {
  function func() {
    return function() {
      return;
    }
  }
  return func()`hello world!`;
}
//CHECK-LABEL:function callExpr()
//CHECK-NEXT:frame = [func]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %func()
//CHECK-NEXT:  %1 = StoreFrameInst %0 : closure, [func]
//CHECK-NEXT:  %2 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 5 : number, true : boolean, "hello world!" : string
//CHECK-NEXT:  %3 = LoadFrameInst [func]
//CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
//CHECK-NEXT:  %5 = CallInst %4, undefined : undefined, %2
//CHECK-NEXT:  %6 = ReturnInst %5
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %7 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

/// Some more test cases to check template object id.

function dup2() {
  return dummy`hello world${1 + 2}!!!`;
}
//CHECK-LABEL:function dup2()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
//CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1 : number, 2 : number
//CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, %0, %1
//CHECK-NEXT:  %4 = ReturnInst %3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function dup3() {
  return dummy`hello world${7}!!!`;
}
//CHECK-LABEL:function dup3()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, %0, 7 : number
//CHECK-NEXT:  %3 = ReturnInst %2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function helloWorld() {
  return dummy`hello${0} world!!!`;
}
//CHECK-LABEL:function helloWorld()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.getTemplateObject] : number, undefined : undefined, 6 : number, true : boolean, "hello" : string, " world!!!" : string
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "dummy" : string
//CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, %0, 0 : number
//CHECK-NEXT:  %3 = ReturnInst %2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

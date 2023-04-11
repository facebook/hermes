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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "dummy": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "emptyString": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "oneString": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "oneSub": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "dup": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "notDup": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "memberExpr": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "callExpr": string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "dup2": string
// CHECK-NEXT:  %9 = DeclareGlobalVarInst "dup3": string
// CHECK-NEXT:  %10 = DeclareGlobalVarInst "helloWorld": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %dummy(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: object, globalObject: object, "dummy": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %emptyString(): any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: object, globalObject: object, "emptyString": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %oneString(): any
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: object, globalObject: object, "oneString": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %oneSub(): any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: object, globalObject: object, "oneSub": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %dup(): any
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19: object, globalObject: object, "dup": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %notDup(): any
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21: object, globalObject: object, "notDup": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %memberExpr(): any
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23: object, globalObject: object, "memberExpr": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %callExpr(): any
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25: object, globalObject: object, "callExpr": string
// CHECK-NEXT:  %27 = CreateFunctionInst (:object) %dup2(): any
// CHECK-NEXT:  %28 = StorePropertyLooseInst %27: object, globalObject: object, "dup2": string
// CHECK-NEXT:  %29 = CreateFunctionInst (:object) %dup3(): any
// CHECK-NEXT:  %30 = StorePropertyLooseInst %29: object, globalObject: object, "dup3": string
// CHECK-NEXT:  %31 = CreateFunctionInst (:object) %helloWorld(): any
// CHECK-NEXT:  %32 = StorePropertyLooseInst %31: object, globalObject: object, "helloWorld": string
// CHECK-NEXT:  %33 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %34 = StoreStackInst undefined: undefined, %33: any
// CHECK-NEXT:  %35 = LoadStackInst (:any) %33: any
// CHECK-NEXT:  %36 = ReturnInst %35: any
// CHECK-NEXT:function_end

// CHECK:function dummy(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function emptyString(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 0: number, true: boolean, "": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function oneString(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 1: number, true: boolean, "hello": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function oneSub(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 2: number, true: boolean, "": string, "": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: any, 666: number
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function dup(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %1 = BinaryAddInst (:any) 1: number, 2: number
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, %0: any, %1: any
// CHECK-NEXT:  %4 = ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function notDup(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 4: number, false: boolean, "hello\\\\nworld": string, "!!!": string, "hello\\nworld": string, "!!!": string
// CHECK-NEXT:  %1 = BinaryAddInst (:any) 1: number, 2: number
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, %0: any, %1: any
// CHECK-NEXT:  %4 = ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function memberExpr(): any
// CHECK-NEXT:frame = [obj: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [obj]: any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) "func": string, %1: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: object, [obj]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "func": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, %5: any, %4: any
// CHECK-NEXT:  %8 = ReturnInst %7: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function callExpr(): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [func]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %func(): any
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [func]: any
// CHECK-NEXT:  %3 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, %3: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function dup2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %1 = BinaryAddInst (:any) 1: number, 2: number
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, %0: any, %1: any
// CHECK-NEXT:  %4 = ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function dup3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: any, 7: number
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function helloWorld(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 6: number, true: boolean, "hello": string, " world!!!": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: any, 0: number
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function func(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

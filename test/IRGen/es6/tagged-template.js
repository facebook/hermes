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
// CHECK-NEXT:       DeclareGlobalVarInst "dummy": string
// CHECK-NEXT:       DeclareGlobalVarInst "emptyString": string
// CHECK-NEXT:       DeclareGlobalVarInst "oneString": string
// CHECK-NEXT:       DeclareGlobalVarInst "oneSub": string
// CHECK-NEXT:       DeclareGlobalVarInst "dup": string
// CHECK-NEXT:       DeclareGlobalVarInst "notDup": string
// CHECK-NEXT:       DeclareGlobalVarInst "memberExpr": string
// CHECK-NEXT:       DeclareGlobalVarInst "callExpr": string
// CHECK-NEXT:       DeclareGlobalVarInst "dup2": string
// CHECK-NEXT:       DeclareGlobalVarInst "dup3": string
// CHECK-NEXT:        DeclareGlobalVarInst "helloWorld": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %dummy(): any
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "dummy": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %emptyString(): any
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "emptyString": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %oneString(): any
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "oneString": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %oneSub(): any
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "oneSub": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %dup(): any
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "dup": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %notDup(): any
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "notDup": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %memberExpr(): any
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "memberExpr": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %callExpr(): any
// CHECK-NEXT:        StorePropertyLooseInst %25: object, globalObject: object, "callExpr": string
// CHECK-NEXT:  %27 = CreateFunctionInst (:object) %dup2(): any
// CHECK-NEXT:        StorePropertyLooseInst %27: object, globalObject: object, "dup2": string
// CHECK-NEXT:  %29 = CreateFunctionInst (:object) %dup3(): any
// CHECK-NEXT:        StorePropertyLooseInst %29: object, globalObject: object, "dup3": string
// CHECK-NEXT:  %31 = CreateFunctionInst (:object) %helloWorld(): any
// CHECK-NEXT:        StorePropertyLooseInst %31: object, globalObject: object, "helloWorld": string
// CHECK-NEXT:  %33 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %33: any
// CHECK-NEXT:  %35 = LoadStackInst (:any) %33: any
// CHECK-NEXT:        ReturnInst %35: any
// CHECK-NEXT:function_end

// CHECK:function dummy(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function emptyString(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 0: number, true: boolean, "": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function oneString(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 1: number, true: boolean, "hello": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function oneSub(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 2: number, true: boolean, "": string, "": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, 666: number
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function dup(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %1 = BinaryAddInst (:any) 1: number, 2: number
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, %1: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function notDup(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 4: number, false: boolean, "hello\\\\nworld": string, "!!!": string, "hello\\nworld": string, "!!!": string
// CHECK-NEXT:  %1 = BinaryAddInst (:any) 1: number, 2: number
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, %1: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function memberExpr(): any
// CHECK-NEXT:frame = [obj: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [obj]: any
// CHECK-NEXT:  %1 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:       StoreNewOwnPropertyInst %2: any, %1: object, "func": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %1: object, [obj]: any
// CHECK-NEXT:  %5 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "func": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, %6: any, %5: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function callExpr(): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [func]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %func(): any
// CHECK-NEXT:       StoreFrameInst %1: object, [func]: any
// CHECK-NEXT:  %3 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [func]: any
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function dup2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %1 = BinaryAddInst (:any) 1: number, 2: number
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, %1: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function dup3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, 7: number
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function helloWorld(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetTemplateObjectInst (:any) 6: number, true: boolean, "hello": string, " world!!!": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, 0: number
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function func(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

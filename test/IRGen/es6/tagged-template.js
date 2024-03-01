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

function dup(x) {
  return dummy`hello world${1 + x}!!!`;
}

function notDup(x) {
  return dummy`hello\nworld${1 + x}!!!`;
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

function dup2(x) {
  return dummy`hello world${1 + x}!!!`;
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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "dummy": string
// CHECK-NEXT:       DeclareGlobalVarInst "emptyString": string
// CHECK-NEXT:       DeclareGlobalVarInst "oneString": string
// CHECK-NEXT:       DeclareGlobalVarInst "oneSub": string
// CHECK-NEXT:       DeclareGlobalVarInst "dup": string
// CHECK-NEXT:       DeclareGlobalVarInst "notDup": string
// CHECK-NEXT:       DeclareGlobalVarInst "memberExpr": string
// CHECK-NEXT:       DeclareGlobalVarInst "callExpr": string
// CHECK-NEXT:       DeclareGlobalVarInst "dup2": string
// CHECK-NEXT:        DeclareGlobalVarInst "dup3": string
// CHECK-NEXT:        DeclareGlobalVarInst "helloWorld": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %dummy(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "dummy": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %emptyString(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "emptyString": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %oneString(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "oneString": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %oneSub(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "oneSub": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %dup(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "dup": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %0: environment, %notDup(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "notDup": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %0: environment, %memberExpr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %24: object, globalObject: object, "memberExpr": string
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %0: environment, %callExpr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %26: object, globalObject: object, "callExpr": string
// CHECK-NEXT:  %28 = CreateFunctionInst (:object) %0: environment, %dup2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %28: object, globalObject: object, "dup2": string
// CHECK-NEXT:  %30 = CreateFunctionInst (:object) %0: environment, %dup3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %30: object, globalObject: object, "dup3": string
// CHECK-NEXT:  %32 = CreateFunctionInst (:object) %0: environment, %helloWorld(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %32: object, globalObject: object, "helloWorld": string
// CHECK-NEXT:  %34 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %34: any
// CHECK-NEXT:  %36 = LoadStackInst (:any) %34: any
// CHECK-NEXT:        ReturnInst %36: any
// CHECK-NEXT:function_end

// CHECK:function dummy(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %dummy(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function emptyString(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %emptyString(): any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 0: number, true: boolean, "": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function oneString(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %oneString(): any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 1: number, true: boolean, "hello": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function oneSub(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %oneSub(): any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 2: number, true: boolean, "": string, "": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: any, 666: number
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function dup(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %dup(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) 1: number, %5: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: any, %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function notDup(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %notDup(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 4: number, false: boolean, "hello\\\\nworld": string, "!!!": string, "hello\\nworld": string, "!!!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) 1: number, %5: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: any, %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function memberExpr(): any
// CHECK-NEXT:frame = [obj: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %memberExpr(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [obj]: any
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:       StoreNewOwnPropertyInst %4: any, %3: object, "func": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [obj]: any
// CHECK-NEXT:  %7 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "func": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, %8: any, %7: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:function callExpr(): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %callExpr(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [func]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %func(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [func]: any
// CHECK-NEXT:  %5 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [func]: any
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function dup2(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %dup2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) 1: number, %5: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: any, %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function dup3(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %dup3(): any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: any, 7: number
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function helloWorld(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %helloWorld(): any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 6: number, true: boolean, "hello": string, " world!!!": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: any, 0: number
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function func(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %callExpr(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %func(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %func(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

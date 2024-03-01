/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O -fno-inline | %FileCheckOrRegen %s

"use strict";

function main(p) {
  function foo(x, y) { return x + y; }
  function bar(x, y) { return x + y; }

  // capture bar.
  p.p = bar;

  return foo(1,2) + bar (1,2)
}

function return_types(p) {
  function builder() { return k * 1 }

  return builder() + builder()
}

function test_unused_and_duplicate_params() {
  function foo2(a, b, c, d) {
    return a + b + c
  }

  function bar1(e) {
    foo2(e ,2, 1, 4, 5, "")
  }

  function bar2(e) {
    foo2(e, 2, 3 ,4 ,5)
  }
  return [bar1, bar2]
}

// Usage of rest arguments should disable signature optimization.
function test_rest_arguments() {
  function baz(...rest) { return rest; }
  return baz(100);
}

function test_generator() {
  function* gen(x) { return x; }
  return gen(1);
}

function test_async() {
  async function asyncFn(x) { return x; }
  return asyncFn(1);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:       DeclareGlobalVarInst "return_types": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_unused_and_duplicate_params": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_rest_arguments": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_generator": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_async": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %main(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %7: object, globalObject: object, "main": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %return_types(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %9: object, globalObject: object, "return_types": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %test_unused_and_duplicate_params(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %11: object, globalObject: object, "test_unused_and_duplicate_params": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %test_rest_arguments(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %13: object, globalObject: object, "test_rest_arguments": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %test_generator(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %15: object, globalObject: object, "test_generator": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %test_async(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %17: object, globalObject: object, "test_async": string
// CHECK-NEXT:        ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(p: any): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %main(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %4: object, %2: any, "p": string
// CHECK-NEXT:  %6 = CallInst (:number) %3: object, %foo(): functionCode, %1: environment, undefined: undefined, 0: number, 1: number, 2: number
// CHECK-NEXT:  %7 = CallInst (:string|number|bigint) %4: object, %bar(): functionCode, %1: environment, undefined: undefined, undefined: undefined, 1: number, 2: number
// CHECK-NEXT:  %8 = BinaryAddInst (:string|number) %6: number, %7: string|number|bigint
// CHECK-NEXT:       ReturnInst %8: string|number
// CHECK-NEXT:function_end

// CHECK:function return_types(p: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %return_types(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %builder(): functionCode
// CHECK-NEXT:  %3 = CallInst (:number) %2: object, %builder(): functionCode, %1: environment, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:number) %2: object, %builder(): functionCode, %1: environment, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = FAddInst (:number) %3: number, %4: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function test_unused_and_duplicate_params(): object
// CHECK-NEXT:frame = [foo2: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_unused_and_duplicate_params(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %foo2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [foo2]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %bar1(): functionCode
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %bar2(): functionCode
// CHECK-NEXT:  %6 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:       StoreOwnPropertyInst %4: object, %6: object, 0: number, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst %5: object, %6: object, 1: number, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function test_rest_arguments(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_rest_arguments(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %baz(): functionCode
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, %baz(): functionCode, %1: environment, undefined: undefined, undefined: undefined, 100: number
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function test_generator(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_generator(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %gen(): functionCode
// CHECK-NEXT:  %3 = CallInst (:object) %2: object, %gen(): functionCode, %1: environment, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function test_async(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_async(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %asyncFn(): functionCode
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, %asyncFn(): functionCode, %1: environment, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: number, y: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = FAddInst (:number) 1: number, 2: number
// CHECK-NEXT:       ReturnInst %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(x: any, y: any): string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:       ReturnInst %2: string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function builder(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "k": string
// CHECK-NEXT:  %1 = BinaryMultiplyInst (:number) %0: any, 1: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function foo2(a: any, b: number, c: number, d: number): string|number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %c: number
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number) %0: any, 2: number
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number) %2: string|number, %1: number
// CHECK-NEXT:       ReturnInst %3: string|number
// CHECK-NEXT:function_end

// CHECK:function bar1(e: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_unused_and_duplicate_params(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [foo2@test_unused_and_duplicate_params]: object
// CHECK-NEXT:  %3 = CallInst (:string|number) %2: object, %foo2(): functionCode, empty: any, undefined: undefined, 0: number, %1: any, 2: number, 1: number, 0: number, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar2(e: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_unused_and_duplicate_params(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [foo2@test_unused_and_duplicate_params]: object
// CHECK-NEXT:  %3 = CallInst (:string|number) %2: object, %foo2(): functionCode, empty: any, undefined: undefined, 0: number, %1: any, 2: number, 3: number, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function baz(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CallBuiltinInst (:any) [HermesBuiltin.copyRestArgs]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end

// CHECK:function gen(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_generator(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %gen(): any, %0: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_gen(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function asyncFn(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:undefined) %<this>: undefined
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %test_async(): any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %asyncFn(): any, %2: environment
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %3: environment, %?anon_0_asyncFn(): functionCode
// CHECK-NEXT:  %5 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %6 = CallInst (:any) %5: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object, %1: undefined, %0: object
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_gen(x: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_asyncFn(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %asyncFn(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %?anon_0_asyncFn(): any, %0: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_?anon_0_asyncFn(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_asyncFn(x: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

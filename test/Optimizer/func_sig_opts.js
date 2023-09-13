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
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:       DeclareGlobalVarInst "return_types": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_unused_and_duplicate_params": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_rest_arguments": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_generator": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_async": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %main(): string|number
// CHECK-NEXT:       StorePropertyStrictInst %6: object, globalObject: object, "main": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %return_types(): number
// CHECK-NEXT:       StorePropertyStrictInst %8: object, globalObject: object, "return_types": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %test_unused_and_duplicate_params(): object
// CHECK-NEXT:        StorePropertyStrictInst %10: object, globalObject: object, "test_unused_and_duplicate_params": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %test_rest_arguments(): any
// CHECK-NEXT:        StorePropertyStrictInst %12: object, globalObject: object, "test_rest_arguments": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %test_generator(): object
// CHECK-NEXT:        StorePropertyStrictInst %14: object, globalObject: object, "test_generator": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %test_async(): any
// CHECK-NEXT:        StorePropertyStrictInst %16: object, globalObject: object, "test_async": string
// CHECK-NEXT:        ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(p: any): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): number
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %bar(): string|number|bigint
// CHECK-NEXT:       StorePropertyStrictInst %2: object, %0: any, "p": string
// CHECK-NEXT:  %4 = CallInst (:number) %1: object, %foo(): number, empty: any, undefined: undefined, 0: number, 1: number, 2: number
// CHECK-NEXT:  %5 = CallInst (:string|number|bigint) %2: object, %bar(): string|number|bigint, empty: any, undefined: undefined, undefined: undefined, 1: number, 2: number
// CHECK-NEXT:  %6 = BinaryAddInst (:string|number) %4: number, %5: string|number|bigint
// CHECK-NEXT:       ReturnInst %6: string|number
// CHECK-NEXT:function_end

// CHECK:function return_types(p: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %builder(): number
// CHECK-NEXT:  %1 = CallInst (:number) %0: object, %builder(): number, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:number) %0: object, %builder(): number, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = FAddInst (:number) %1: number, %2: number
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function test_unused_and_duplicate_params(): object
// CHECK-NEXT:frame = [foo2: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo2(): string|number
// CHECK-NEXT:       StoreFrameInst %0: object, [foo2]: object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %bar1(): undefined
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %bar2(): undefined
// CHECK-NEXT:  %4 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:       StoreOwnPropertyInst %2: object, %4: object, 0: number, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst %3: object, %4: object, 1: number, true: boolean
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function test_rest_arguments(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %baz(): any
// CHECK-NEXT:  %1 = CallInst (:any) %0: object, %baz(): any, empty: any, undefined: undefined, undefined: undefined, 100: number
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:function test_generator(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %gen(): object
// CHECK-NEXT:  %1 = CallInst (:object) %0: object, %gen(): object, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function test_async(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %asyncFn(): any
// CHECK-NEXT:  %1 = CallInst (:any) %0: object, %asyncFn(): any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst %1: any
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
// CHECK-NEXT:  %0 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [foo2@test_unused_and_duplicate_params]: object
// CHECK-NEXT:  %2 = CallInst (:string|number) %1: object, %foo2(): string|number, empty: any, undefined: undefined, 0: number, %0: any, 2: number, 1: number, 0: number, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar2(e: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [foo2@test_unused_and_duplicate_params]: object
// CHECK-NEXT:  %2 = CallInst (:string|number) %1: object, %foo2(): string|number, empty: any, undefined: undefined, 0: number, %0: any, 2: number, 3: number, 0: number, 0: number
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
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_gen(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function asyncFn(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:undefined) %<this>: undefined
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %?anon_0_asyncFn(): object
// CHECK-NEXT:  %3 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %4 = CallInst (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: object, %1: undefined, %0: object
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_gen(x: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_asyncFn(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_asyncFn(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_asyncFn(x: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

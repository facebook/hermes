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

// CHECK:scope %VS0 []

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %3: object, %1: any, "p": string
// CHECK-NEXT:  %5 = CallInst (:number) %2: object, %foo(): functionCode, empty: any, undefined: undefined, 0: number, 1: number, 2: number
// CHECK-NEXT:  %6 = CallInst (:string|number|bigint) %3: object, %bar(): functionCode, empty: any, undefined: undefined, undefined: undefined, 1: number, 2: number
// CHECK-NEXT:  %7 = BinaryAddInst (:string|number) %5: number, %6: string|number|bigint
// CHECK-NEXT:       ReturnInst %7: string|number
// CHECK-NEXT:function_end

// CHECK:function return_types(p: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %builder(): functionCode
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %builder(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:number) %1: object, %builder(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = FAddInst (:number) %2: number, %3: number
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [foo2: object]

// CHECK:function test_unused_and_duplicate_params(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %foo2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS1.foo2]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %bar1(): functionCode
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %bar2(): functionCode
// CHECK-NEXT:  %6 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:       StoreOwnPropertyInst %4: object, %6: object, 0: number, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst %5: object, %6: object, 1: number, true: boolean
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function test_rest_arguments(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %baz(): functionCode
// CHECK-NEXT:  %2 = CallInst (:any) %1: object, %baz(): functionCode, empty: any, undefined: undefined, undefined: undefined, 100: number
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function test_generator(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %gen(): functionCode
// CHECK-NEXT:  %2 = CallInst (:object) %1: object, %gen(): functionCode, %0: environment, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function test_async(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %asyncFn(): functionCode
// CHECK-NEXT:  %2 = CallInst (:any) %1: object, %asyncFn(): functionCode, %0: environment, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: number, y: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = FAddInst (:number) 1: number, 2: number
// CHECK-NEXT:       ReturnInst %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(x: any, y: any): string|number|bigint
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:       ReturnInst %2: string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function builder(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "k": string
// CHECK-NEXT:  %1 = BinaryMultiplyInst (:number) %0: any, 1: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function foo2(a: any, b: number, c: number, d: number): string|number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %c: number
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number) %0: any, 2: number
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number) %2: string|number, %1: number
// CHECK-NEXT:       ReturnInst %3: string|number
// CHECK-NEXT:function_end

// CHECK:function bar1(e: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS1.foo2]: object
// CHECK-NEXT:  %3 = CallInst (:string|number) %2: object, %foo2(): functionCode, empty: any, undefined: undefined, 0: number, %1: any, 2: number, 1: number, 0: number, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar2(e: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %e: any
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS1.foo2]: object
// CHECK-NEXT:  %3 = CallInst (:string|number) %2: object, %foo2(): functionCode, empty: any, undefined: undefined, 0: number, %1: any, 2: number, 3: number, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function baz(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CallBuiltinInst (:any) [HermesBuiltin.copyRestArgs]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: number, generator_state: number]

// CHECK:function gen(x: number): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [%VS2.x]: number
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS2.generator_state]: number
// CHECK-NEXT:  %5 = CreateGeneratorInst (:object) %1: environment, %gen?inner(): functionCode
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:function asyncFn(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsStrictInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:undefined) %<this>: undefined
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %2: environment, %?anon_0_asyncFn(): functionCode
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object, %1: undefined, %0: object
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:generator inner gen?inner(action: number, value: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %value: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %action: number
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:number) %2: environment, [%VS2.generator_state]: number
// CHECK-NEXT:       CmpBrStrictlyEqualInst %3: number, 2: number, %BB6, %BB7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:number) %2: environment, [%VS2.x]: number
// CHECK-NEXT:       StoreFrameInst %2: environment, 3: number, [%VS2.generator_state]: number
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) empty: any, "value": string, %5: number, "done": string, true: boolean
// CHECK-NEXT:       ReturnInst %7: object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst %2: environment, 3: number, [%VS2.generator_state]: number
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %10: object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS2.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB2, %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %2: environment, 2: number, [%VS2.generator_state]: number
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS2.generator_state]: number
// CHECK-NEXT:        ThrowTypeErrorInst "Generator functions may not be called on executing generators": string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %3: number, 3: number, %BB8, %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB9, %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 2: number, %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %24 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %24: object
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %26 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %26: object
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any, generator_state: number]

// CHECK:function ?anon_0_asyncFn(x: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS3.generator_state]: number
// CHECK-NEXT:  %5 = CreateGeneratorInst (:object) %1: environment, %?anon_0_asyncFn?inner(): functionCode
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:generator inner ?anon_0_asyncFn?inner(action: number, value: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %value: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %action: number
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:number) %2: environment, [%VS3.generator_state]: number
// CHECK-NEXT:       CmpBrStrictlyEqualInst %3: number, 2: number, %BB6, %BB7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [%VS3.x]: any
// CHECK-NEXT:       StoreFrameInst %2: environment, 3: number, [%VS3.generator_state]: number
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) empty: any, "value": string, %5: any, "done": string, true: boolean
// CHECK-NEXT:       ReturnInst %7: object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst %2: environment, 3: number, [%VS3.generator_state]: number
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %10: object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS3.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB2, %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %2: environment, 2: number, [%VS3.generator_state]: number
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS3.generator_state]: number
// CHECK-NEXT:        ThrowTypeErrorInst "Generator functions may not be called on executing generators": string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %3: number, 3: number, %BB8, %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB9, %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 2: number, %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %24 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %24: object
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %26 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %26: object
// CHECK-NEXT:function_end

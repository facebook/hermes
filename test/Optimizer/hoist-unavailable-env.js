/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test our ability to hoist functions where their enclosing environment has to
// retrieved via GetClosureScopeInst.

function foo(sink){
  let f1, f2;
  let numBar = 0
  function bar() {
    let numF1 = ++numBar;

    function baz() {
      let numF2 = ++numBaz;
      // Assign f1 and f2 here, so the parent environment of the function is
      // unavailable when we call them, and we are forced to get the environment
      // from the closure.
      f1 = function () {
        "inline";
        // Since f1 only uses numF1, it can be hoisted to the scope of bar.
        return ++numF1;
      };
      f2 = function () {
        "inline";
        // f2 must remain at the scope of baz, since it uses numF2.
        return ++numF2;
      };
    }
    return baz;
  }
  sink(bar);
  return f1() + f2();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [f1: undefined|object, f2: undefined|object, numBar: number]

// CHECK:scope %VS1 [numF2: undefined|number|bigint]

// CHECK:scope %VS2 [numF1: number]

// CHECK:function foo(sink: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %bar(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS0.f1]: undefined|object
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS0.f2]: undefined|object
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS0.numBar]: number
// CHECK-NEXT:  %6 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:  %7 = LoadFrameInst (:undefined|object) %0: environment, [%VS0.f1]: undefined|object
// CHECK-NEXT:  %8 = TypeOfIsInst (:boolean) %7: undefined|object, typeOfIs(Function)
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB4, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ThrowTypeErrorInst "Trying to call a non-function": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = GetClosureScopeInst (:environment) %VS1: any, %f2(): functionCode, %22: undefined|object
// CHECK-NEXT:  %12 = LoadFrameInst (:undefined|number|bigint) %11: environment, [%VS1.numF2]: undefined|number|bigint
// CHECK-NEXT:  %13 = UnaryIncInst (:number|bigint) %12: undefined|number|bigint
// CHECK-NEXT:        StoreFrameInst %11: environment, %13: number|bigint, [%VS1.numF2]: undefined|number|bigint
// CHECK-NEXT:  %15 = BinaryAddInst (:number) %20: number, %13: number|bigint
// CHECK-NEXT:        ReturnInst %15: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ThrowTypeErrorInst "Trying to call a non-function": string
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = GetClosureScopeInst (:environment) %VS2: any, %f1(): functionCode, %7: undefined|object
// CHECK-NEXT:  %19 = LoadFrameInst (:number) %18: environment, [%VS2.numF1]: number
// CHECK-NEXT:  %20 = FAddInst (:number) %19: number, 1: number
// CHECK-NEXT:        StoreFrameInst %18: environment, %20: number, [%VS2.numF1]: number
// CHECK-NEXT:  %22 = LoadFrameInst (:undefined|object) %0: environment, [%VS0.f2]: undefined|object
// CHECK-NEXT:  %23 = TypeOfIsInst (:boolean) %22: undefined|object, typeOfIs(Function)
// CHECK-NEXT:        CondBranchInst %23: boolean, %BB2, %BB1
// CHECK-NEXT:function_end

// CHECK:function bar(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %VS2: any, %baz(): functionCode
// CHECK-NEXT:  %3 = LoadFrameInst (:number) %0: environment, [%VS0.numBar]: number
// CHECK-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %4: number, [%VS0.numBar]: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: number, [%VS2.numF1]: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function baz(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.numF2]: undefined|number|bigint
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "numBaz": string
// CHECK-NEXT:  %4 = UnaryIncInst (:number|bigint) %3: any
// CHECK-NEXT:       StorePropertyLooseInst %4: number|bigint, globalObject: object, "numBaz": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: number|bigint, [%VS1.numF2]: undefined|number|bigint
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS2: any, %f1(): functionCode
// CHECK-NEXT:  %8 = ResolveScopeInst (:environment) %VS0: any, %VS2: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %8: environment, %7: object, [%VS0.f1]: undefined|object
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %VS1: any, %f2(): functionCode
// CHECK-NEXT:        StoreFrameInst %8: environment, %10: object, [%VS0.f2]: undefined|object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f1(): number [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function f2(): number|bigint [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

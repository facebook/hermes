/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s

function bar() { }

// The first stores to k1, k2, k3 can be eliminated since the variables haven't
// been captured yet.
function main(p) {
  var k1 = bar();
  var k2 = bar();
  var k3 = bar();

  return function () { return k1 + k2 + k3 }
}

// Make sure that we are not eliminating the "store-42" in test2, because the
// call to o() may clobber it.
function outer() {
    var envVar;

    function setValue(v) {
        envVar = v;
    }

    function test1() {
      envVar = 42;
      envVar = 87;
    }

    function test2(o) {
      envVar = 42;
      o();
      envVar = 87;
    }

    return [setValue, test1, test2, envVar]
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "bar": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %bar(): undefined
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3: closure, globalObject: object, "bar": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %main(): closure
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: closure, globalObject: object, "main": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %outer(): object
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: closure, globalObject: object, "outer": string
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(p: any): closure
// CHECK-NEXT:frame = [k1: any, k2: any, k3: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst (:any) globalObject: object, "bar": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %2 = StoreFrameInst %1: any, [k1]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "bar": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [k2]: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "bar": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %8 = StoreFrameInst %7: any, [k3]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %""(): string|number|bigint
// CHECK-NEXT:  %10 = ReturnInst %9: closure
// CHECK-NEXT:function_end

// CHECK:function outer(): object
// CHECK-NEXT:frame = [envVar: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [envVar]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %setValue(): undefined
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %test1(): undefined
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %test2(): undefined
// CHECK-NEXT:  %4 = AllocArrayInst (:object) 4: number
// CHECK-NEXT:  %5 = StoreOwnPropertyInst %1: closure, %4: object, 0: number, true: boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst %2: closure, %4: object, 1: number, true: boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst %3: closure, %4: object, 2: number, true: boolean
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [envVar]: any
// CHECK-NEXT:  %9 = StoreOwnPropertyInst %8: any, %4: object, 3: number, true: boolean
// CHECK-NEXT:  %10 = ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function ""(): string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [k1@main]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [k2@main]: any
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [k3@main]: any
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number|bigint) %2: string|number|bigint, %3: any
// CHECK-NEXT:  %5 = ReturnInst %4: string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function setValue(v: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %v: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [envVar@outer]: any
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test1(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 87: number, [envVar@outer]: any
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test2(o: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %1 = StoreFrameInst 42: number, [envVar@outer]: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %3 = StoreFrameInst 87: number, [envVar@outer]: any
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

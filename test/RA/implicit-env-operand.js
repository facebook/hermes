/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-lra -O0 -Xg3 -Xes6-block-scoping %s | %FileCheckOrRegen %s --match-full-lines

(function () {
  var v1 = 0;
  function foo() {
    let v2 = 12;
    while (true) {
      class B {} // Class declaration forces while loop to create a new scope / environment.
      let v3 = 42;
      print(v3);
      print(v2 + 10);
      print(v1 + 20);
    }
  }
  foo();
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = EvalCompilationDataInst empty: any, undefined: undefined, empty: any, empty: any, empty: any, empty: any, %VS0: any
// CHECK-NEXT:  {r2}      %1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  {r1}      %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  {r3}      %3 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r1}      %4 = MovInst (:undefined) {r3} %3: undefined
// CHECK-NEXT:  {r3}      %5 = CreateFunctionInst (:object) {r2} %1: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  {r4}      %6 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r5}      %7 = ImplicitMovInst (:undefined) {r4} %6: undefined
// CHECK-NEXT:  {r3}      %8 = HBCCallNInst (:any) {r3} %5: object, empty: any, false: boolean, empty: any, undefined: undefined, {r4} %6: undefined
// CHECK-NEXT:  {r1}      %9 = MovInst (:any) {r3} %8: any
// CHECK-NEXT:  {r1}     %10 = LoadStackInst (:any) {r1} %2: any
// CHECK-NEXT:  {r0}     %11 = ReturnInst {r1} %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [v1: any, foo: any]

// CHECK:scope %VS1 []

// CHECK:function ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = EvalCompilationDataInst empty: any, undefined: undefined, empty: any, empty: any, empty: any, empty: any, %VS0: any, %VS1: any
// CHECK-NEXT:  {r2}      %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  {r2}      %2 = CreateScopeInst (:environment) %VS0: any, {r2} %1: environment
// CHECK-NEXT:  {r1}      %3 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}      %4 = StoreFrameInst {r2} %2: environment, {r1} %3: undefined, [%VS0.v1]: any
// CHECK-NEXT:  {r1}      %5 = CreateFunctionInst (:object) {r2} %2: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:  {r0}      %6 = StoreFrameInst {r2} %2: environment, {r1} %5: object, [%VS0.foo]: any
// CHECK-NEXT:  {r1}      %7 = LIRLoadConstInst (:number) 0: number
// CHECK-NEXT:  {r0}      %8 = StoreFrameInst {r2} %2: environment, {r1} %7: number, [%VS0.v1]: any
// CHECK-NEXT:  {r1}      %9 = LoadFrameInst (:any) {r2} %2: environment, [%VS0.foo]: any
// CHECK-NEXT:  {r3}     %10 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r4}     %11 = ImplicitMovInst (:undefined) {r3} %10: undefined
// CHECK-NEXT:  {r0}     %12 = HBCCallNInst (:any) {r1} %9: any, empty: any, false: boolean, empty: any, undefined: undefined, {r3} %10: undefined
// CHECK-NEXT:  {r1}     %13 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %14 = ReturnInst {r1} %13: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [v2: any]

// CHECK:scope %VS1 [v1: any, foo: any]

// CHECK:scope %VS2 []

// CHECK:scope %VS3 [B: any, v3: any, B#1: any, ?B.prototype: object, ?B: object]

// CHECK:function foo(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = EvalCompilationDataInst empty: any, undefined: undefined, empty: any, empty: any, empty: any, empty: any, %VS0: any, %VS1: any, %VS2: any
// CHECK-NEXT:  {r6}      %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  {r2}      %2 = CreateScopeInst (:environment) %VS0: any, {r6} %1: environment
// CHECK-NEXT:  {r5}      %3 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}      %4 = StoreFrameInst {r2} %2: environment, {r5} %3: undefined, [%VS0.v2]: any
// CHECK-NEXT:  {r5}      %5 = LIRLoadConstInst (:number) 12: number
// CHECK-NEXT:  {r0}      %6 = StoreFrameInst {r2} %2: environment, {r5} %5: number, [%VS0.v2]: any
// CHECK-NEXT:  {r5}      %7 = LIRLoadConstInst (:boolean) true: boolean
// CHECK-NEXT:  {r0}      %8 = CondBranchInst {r5} %7: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {r5}      %9 = CreateScopeInst (:environment) %VS3: any, {r2} %2: environment
// CHECK-NEXT:  {r4}     %10 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %11 = StoreFrameInst {r5} %9: environment, {r4} %10: undefined, [%VS3.B]: any
// CHECK-NEXT:  {r4}     %12 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %13 = StoreFrameInst {r5} %9: environment, {r4} %12: undefined, [%VS3.v3]: any
// CHECK-NEXT:  {r4}     %14 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %15 = StoreFrameInst {r5} %9: environment, {r4} %14: undefined, [%VS3.B#1]: any
// CHECK-NEXT:  {r4}     %16 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  {r1}     %17 = CreateClassInst (:object) {r5} %9: environment, %VS3: any, %B(): functionCode, empty: any, {r4} %16: object
// CHECK-NEXT:  {r4}     %18 = LoadStackInst (:object) {r4} %16: object
// CHECK-NEXT:  {r0}     %19 = StoreFrameInst {r5} %9: environment, {r1} %17: object, [%VS3.B#1]: any
// CHECK-NEXT:  {r0}     %20 = StoreFrameInst {r5} %9: environment, {r1} %17: object, [%VS3.?B]: object
// CHECK-NEXT:  {r0}     %21 = StoreFrameInst {r5} %9: environment, {r4} %18: object, [%VS3.?B.prototype]: object
// CHECK-NEXT:  {r0}     %22 = StoreFrameInst {r5} %9: environment, {r1} %17: object, [%VS3.B]: any
// CHECK-NEXT:  {r1}     %23 = LIRLoadConstInst (:number) 42: number
// CHECK-NEXT:  {r0}     %24 = StoreFrameInst {r5} %9: environment, {r1} %23: number, [%VS3.v3]: any
// CHECK-NEXT:  {r1}     %25 = LIRGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r1}     %26 = TryLoadGlobalPropertyInst (:any) {r1} %25: object, "print": string
// CHECK-NEXT:  {r4}     %27 = LoadFrameInst (:any) {r5} %9: environment, [%VS3.v3]: any
// CHECK-NEXT:  {r3}     %28 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r8}     %29 = ImplicitMovInst (:undefined) {r3} %28: undefined
// CHECK-NEXT:  {r7}     %30 = ImplicitMovInst (:any) {r4} %27: any
// CHECK-NEXT:  {r0}     %31 = HBCCallNInst (:any) {r1} %26: any, empty: any, false: boolean, empty: any, undefined: undefined, {r3} %28: undefined, {r4} %27: any
// CHECK-NEXT:  {r1}     %32 = LIRGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r1}     %33 = TryLoadGlobalPropertyInst (:any) {r1} %32: object, "print": string
// CHECK-NEXT:  {r3}     %34 = LoadFrameInst (:any) {r2} %2: environment, [%VS0.v2]: any
// CHECK-NEXT:  {r4}     %35 = LIRLoadConstInst (:number) 10: number
// CHECK-NEXT:  {r3}     %36 = BinaryAddInst (:any) {r3} %34: any, {r4} %35: number
// CHECK-NEXT:  {r4}     %37 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r8}     %38 = ImplicitMovInst (:undefined) {r4} %37: undefined
// CHECK-NEXT:  {r7}     %39 = ImplicitMovInst (:any) {r3} %36: any
// CHECK-NEXT:  {r0}     %40 = HBCCallNInst (:any) {r1} %33: any, empty: any, false: boolean, empty: any, undefined: undefined, {r4} %37: undefined, {r3} %36: any
// CHECK-NEXT:  {r1}     %41 = LIRGetGlobalObjectInst (:object)
// CHECK-NEXT:  {r1}     %42 = TryLoadGlobalPropertyInst (:any) {r1} %41: object, "print": string
// CHECK-NEXT:  {r4}     %43 = LoadFrameInst (:any) {r6} %1: environment, [%VS1.v1]: any
// CHECK-NEXT:  {r3}     %44 = LIRLoadConstInst (:number) 20: number
// CHECK-NEXT:  {r4}     %45 = BinaryAddInst (:any) {r4} %43: any, {r3} %44: number
// CHECK-NEXT:  {r3}     %46 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r8}     %47 = ImplicitMovInst (:undefined) {r3} %46: undefined
// CHECK-NEXT:  {r7}     %48 = ImplicitMovInst (:any) {r4} %45: any
// CHECK-NEXT:  {r0}     %49 = HBCCallNInst (:any) {r1} %42: any, empty: any, false: boolean, empty: any, undefined: undefined, {r3} %46: undefined, {r4} %45: any
// CHECK-NEXT:  {r0}     %50 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {r1}     %51 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {r0}     %52 = ReturnInst {r1} %51: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {r1}     %53 = LIRLoadConstInst (:boolean) true: boolean
// CHECK-NEXT:  {r0}     %54 = CondBranchInst {r1} %53: boolean, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [this: object]

// CHECK:scope %VS1 [B: any, v3: any, B#1: any, ?B.prototype: object, ?B: object]

// CHECK:scope %VS2 [v2: any]

// CHECK:scope %VS3 [v1: any, foo: any]

// CHECK:scope %VS4 []

// CHECK:base constructor B(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = EvalCompilationDataInst empty: any, undefined: undefined, empty: any, empty: any, [%VS1.?B]: object, empty: any, %VS0: any, %VS1: any, %VS2: any, %VS3: any, %VS4: any
// CHECK-NEXT:  {r2}      %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  {r2}      %2 = CreateScopeInst (:environment) %VS0: any, {r2} %1: environment
// CHECK-NEXT:  {r1}      %3 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  {r1}      %4 = LoadPropertyInst (:any) {r1} %3: object, "prototype": string
// CHECK-NEXT:  {r1}      %5 = AllocObjectLiteralInst (:object) {r1} %4: any
// CHECK-NEXT:  {r0}      %6 = StoreFrameInst {r2} %2: environment, {r1} %5: object, [%VS0.this]: object
// CHECK-NEXT:  {r1}      %7 = LoadFrameInst (:object) {r2} %2: environment, [%VS0.this]: object
// CHECK-NEXT:  {r0}      %8 = ReturnInst {r1} %7: object
// CHECK-NEXT:function_end

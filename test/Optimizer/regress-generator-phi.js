/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 -Xcustom-opt=simplestackpromotion,simplemem2reg,lowergeneratorfunction | %FileCheckOrRegen %s --match-full-lines

function* foo(f, a, b) {
  'noinline';
  a = [];
  if (f) a = yield 1;

  // Create an extra BB after the Yield so that there's a Phi that doesn't
  // have a Yield as a predecessor.
  a = f ? a : b;

  // 'a' is potentially the original [].
  // There was a bug in which the value operand to the Phi didn't dominate
  // the Phi but the Phi operand wasn't being moved to the outer function.
  // This resulted in bad IR after LowerGeneratorFunction.
  print(a);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [<this>: any, f: any, a: any, b: any, ?anon_0_isReturn_prologue: boolean, ?anon_1_isReturn: boolean, generator_state: number, return_value: any, idx: number, catchVal: any, exception_handler_idx: number, PhiInst: any, PhiInst#1: any, LoadFrameInst: any, LoadFrameInst#1: any]

// CHECK:function foo(f: any, a: any, b: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS1: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS1.<this>]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %f: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS1.f]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %7: any, [%VS1.a]: any
// CHECK-NEXT:  %9 = LoadParamInst (:any) %b: any
// CHECK-NEXT:        StoreFrameInst %2: environment, %9: any, [%VS1.b]: any
// CHECK-NEXT:        StoreFrameInst %2: environment, 0: number, [%VS1.idx]: number
// CHECK-NEXT:        StoreFrameInst %2: environment, 0: number, [%VS1.generator_state]: number
// CHECK-NEXT:        StoreFrameInst %2: environment, 0: number, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:  %14 = CreateGeneratorInst (:object) %2: environment, %VS1: any, %"foo 1#"(): functionCode
// CHECK-NEXT:        ReturnInst %14: object
// CHECK-NEXT:function_end

// CHECK:generator inner "foo 1#"(action: number, value: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %value: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %action: number
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:number) %2: environment, [%VS1.generator_state]: number
// CHECK-NEXT:  %4 = BinaryStrictlyEqualInst (:any) %3: number, 2: number
// CHECK-NEXT:       CondBranchInst %4: any, %BB19, %BB20
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %7 = LoadFrameInst (:boolean) %2: environment, [%VS1.?anon_0_isReturn_prologue]: boolean
// CHECK-NEXT:       StoreStackInst %7: boolean, %6: boolean
// CHECK-NEXT:  %9 = BinaryStrictlyEqualInst (:any) %1: number, 1: number
// CHECK-NEXT:        CondBranchInst %9: any, %BB13, %BB14
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = ResolveScopeInst (:environment) %VS0: any, %VS1: any, %2: environment
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS1.f]: any
// CHECK-NEXT:        StoreFrameInst %2: environment, %12: any, [%VS1.LoadFrameInst]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %2: environment, [%VS1.a]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %2: environment, [%VS1.b]: any
// CHECK-NEXT:        StoreFrameInst %2: environment, %15: any, [%VS1.LoadFrameInst#1]: any
// CHECK-NEXT:  %17 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:        CondBranchInst %12: any, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB27, %BB28
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %2: environment, 1: number, [%VS1.idx]: number
// CHECK-NEXT:        StoreFrameInst %2: environment, 1: number, [%VS1.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB27, %BB29
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %2: environment, %17: object, [%VS1.PhiInst]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %2: environment, [%VS1.LoadFrameInst]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) %2: environment, [%VS1.PhiInst]: any
// CHECK-NEXT:        CondBranchInst %26: any, %BB11, %BB10
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %29 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:  %30 = LoadFrameInst (:boolean) %2: environment, [%VS1.?anon_1_isReturn]: boolean
// CHECK-NEXT:        StoreStackInst %30: boolean, %29: boolean
// CHECK-NEXT:  %32 = BinaryStrictlyEqualInst (:any) %1: number, 1: number
// CHECK-NEXT:        CondBranchInst %32: any, %BB15, %BB16
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        StoreFrameInst %2: environment, %61: any, [%VS1.PhiInst]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB27, %BB30
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %38 = LoadFrameInst (:any) %2: environment, [%VS1.LoadFrameInst#1]: any
// CHECK-NEXT:        StoreFrameInst %2: environment, %38: any, [%VS1.PhiInst#1]: any
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StoreFrameInst %2: environment, %27: any, [%VS1.PhiInst#1]: any
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %43 = LoadFrameInst (:any) %2: environment, [%VS1.PhiInst#1]: any
// CHECK-NEXT:  %44 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %45 = CallInst (:any) %44: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %43: any
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB27, %BB31
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any, %BB27
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StoreFrameInst %2: environment, %0: any, [%VS1.return_value]: any
// CHECK-NEXT:  %51 = LoadFrameInst (:any) %2: environment, [%VS1.return_value]: any
// CHECK-NEXT:  %52 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        StoreStackInst %52: boolean, %6: boolean
// CHECK-NEXT:  %54 = LoadStackInst (:boolean) %6: boolean
// CHECK-NEXT:        StoreFrameInst %2: environment, %54: boolean, [%VS1.?anon_0_isReturn_prologue]: boolean
// CHECK-NEXT:  %56 = LoadFrameInst (:boolean) %2: environment, [%VS1.?anon_0_isReturn_prologue]: boolean
// CHECK-NEXT:        CondBranchInst %56: boolean, %BB3, %BB2
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any, %BB27
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        StoreFrameInst %2: environment, %0: any, [%VS1.return_value]: any
// CHECK-NEXT:  %61 = LoadFrameInst (:any) %2: environment, [%VS1.return_value]: any
// CHECK-NEXT:  %62 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        StoreStackInst %62: boolean, %29: boolean
// CHECK-NEXT:  %64 = LoadStackInst (:boolean) %29: boolean
// CHECK-NEXT:        StoreFrameInst %2: environment, %64: boolean, [%VS1.?anon_1_isReturn]: boolean
// CHECK-NEXT:  %66 = LoadFrameInst (:boolean) %2: environment, [%VS1.?anon_1_isReturn]: boolean
// CHECK-NEXT:        CondBranchInst %66: boolean, %BB9, %BB8
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryStartInst %BB27, %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        StoreFrameInst %2: environment, 2: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %70 = LoadFrameInst (:number) %2: environment, [%VS1.idx]: number
// CHECK-NEXT:        SwitchInst %70: number, %BB7, 0: number, %BB1
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowTypeErrorInst "Generator functions may not be called on executing generators": string
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %74 = LoadFrameInst (:number) %2: environment, [%VS1.generator_state]: number
// CHECK-NEXT:  %75 = BinaryStrictlyEqualInst (:any) %74: number, 3: number
// CHECK-NEXT:        CondBranchInst %75: any, %BB21, %BB17
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %77 = BinaryStrictlyEqualInst (:any) %1: number, 1: number
// CHECK-NEXT:        CondBranchInst %77: any, %BB22, %BB23
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %80 = BinaryStrictlyEqualInst (:any) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %80: any, %BB24, %BB25
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %82 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %82: object
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %84 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %84: object
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %87 = LoadFrameInst (:any) %2: environment, [%VS1.catchVal]: any
// CHECK-NEXT:        ThrowInst %87: any
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %89 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %2: environment, %89: any, [%VS1.catchVal]: any
// CHECK-NEXT:  %91 = LoadFrameInst (:number) %2: environment, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %93 = AllocObjectLiteralInst (:object) empty: any, "value": string, %51: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %93: object
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %95 = AllocObjectLiteralInst (:object) empty: any, "value": string, 1: number, "done": string, false: boolean
// CHECK-NEXT:        ReturnInst %95: object
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %97 = AllocObjectLiteralInst (:object) empty: any, "value": string, %61: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %97: object
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %99 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:         ReturnInst %99: object
// CHECK-NEXT:function_end

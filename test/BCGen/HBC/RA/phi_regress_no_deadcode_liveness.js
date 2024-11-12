/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O --target=HBC -dump-ra %s | %FileCheckOrRegen %s --check-prefix=CHKRA --match-full-lines
// Ensure that phi node does not update deadcode liveness interval
function b(d=([[[[{z:[{}]}]]]]=arguments)) {}

// Auto-generated content below. Please do not modify manually.

// CHKRA:scope %VS0 []

// CHKRA:function global(): undefined
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:                 DeclareGlobalVarInst "b": string
// CHKRA-NEXT:  {r0}      %1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKRA-NEXT:  {r1}      %2 = CreateFunctionInst (:object) {r0} %1: environment, %b(): functionCode
// CHKRA-NEXT:  {r0}      %3 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:                 StorePropertyLooseInst {r1} %2: object, {r0} %3: object, "b": string
// CHKRA-NEXT:  {np0}     %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:                 ReturnInst {np0} %5: undefined
// CHKRA-NEXT:function_end

// CHKRA:function b(d: any): undefined
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  {r3}      %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKRA-NEXT:  {r0}      %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:                 StoreStackInst {r0} %1: undefined, {r3} %0: undefined|object
// CHKRA-NEXT:  {r1}      %3 = LoadParamInst (:any) %d: any
// CHKRA-NEXT:                 CmpBrStrictlyNotEqualInst {r1} %3: any, {r0} %1: undefined, %BB2, %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  {r2}      %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHKRA-NEXT:  {r1}      %6 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHKRA-NEXT:                 HBCReifyArgumentsLooseInst {r3} %0: undefined|object
// CHKRA-NEXT:  {r3}      %8 = LoadStackInst (:undefined|object) {r3} %0: undefined|object
// CHKRA-NEXT:  {r3}      %9 = UnionNarrowTrustedInst (:object) {r3} %8: undefined|object
// CHKRA-NEXT:                 StoreStackInst {r3} %9: object, {r1} %6: any
// CHKRA-NEXT:  {r3}     %11 = IteratorBeginInst (:any) {r1} %6: any
// CHKRA-NEXT:                 StoreStackInst {r3} %11: any, {r2} %5: any
// CHKRA-NEXT:                 TryStartInst %BB4, %BB6
// CHKRA-NEXT:%BB2:
// CHKRA-NEXT:                 ReturnInst {r0} %1: undefined
// CHKRA-NEXT:%BB3:
// CHKRA-NEXT:  {r1}     %15 = PhiInst (:any) {r1} %21: any, %BB4, {r1} %37: any, %BB9
// CHKRA-NEXT:  {r3}     %16 = PhiInst (:undefined|boolean) {r3} %22: undefined, %BB4, {r3} %38: boolean, %BB9
// CHKRA-NEXT:  {r1}     %17 = MovInst (:any) {r1} %15: any
// CHKRA-NEXT:  {r3}     %18 = MovInst (:undefined|boolean) {r3} %16: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {r3} %18: undefined|boolean, %BB68, %BB67
// CHKRA-NEXT:%BB4:
// CHKRA-NEXT:  {r4}     %20 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %21 = MovInst (:any) {r4} %20: any
// CHKRA-NEXT:  {r3}     %22 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB3
// CHKRA-NEXT:%BB5:
// CHKRA-NEXT:  {r1}     %24 = LoadStackInst (:any) {r1} %6: any
// CHKRA-NEXT:  {r1}     %25 = IteratorNextInst (:any) {r2} %5: any, {r1} %24: any
// CHKRA-NEXT:  {r3}     %26 = LoadStackInst (:any) {r2} %5: any
// CHKRA-NEXT:  {r3}     %27 = BinaryStrictlyEqualInst (:boolean) {r3} %26: any, {r0} %1: undefined
// CHKRA-NEXT:  {r5}     %28 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {r3} %27: boolean, %BB8, %BB7
// CHKRA-NEXT:%BB6:
// CHKRA-NEXT:                 TryEndInst %BB4, %BB5
// CHKRA-NEXT:%BB7:
// CHKRA-NEXT:  {r5}     %31 = MovInst (:any) {r1} %25: any
// CHKRA-NEXT:                 BranchInst %BB8
// CHKRA-NEXT:%BB8:
// CHKRA-NEXT:  {r5}     %33 = PhiInst (:any) {r5} %28: undefined, %BB5, {r5} %31: any, %BB7
// CHKRA-NEXT:  {r5}     %34 = MovInst (:any) {r5} %33: any
// CHKRA-NEXT:                 TryStartInst %BB9, %BB11
// CHKRA-NEXT:%BB9:
// CHKRA-NEXT:  {r1}     %36 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %37 = MovInst (:any) {r1} %36: any
// CHKRA-NEXT:  {r3}     %38 = MovInst (:boolean) {r3} %27: boolean
// CHKRA-NEXT:                 BranchInst %BB3
// CHKRA-NEXT:%BB10:
// CHKRA-NEXT:                 CondBranchInst {r3} %27: boolean, %BB2, %BB66
// CHKRA-NEXT:%BB11:
// CHKRA-NEXT:  {r4}     %41 = AllocStackInst (:any) $?anon_5_iter: any
// CHKRA-NEXT:  {r1}     %42 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r5} %34: any, {r1} %42: any
// CHKRA-NEXT:  {r5}     %44 = IteratorBeginInst (:any) {r1} %42: any
// CHKRA-NEXT:                 StoreStackInst {r5} %44: any, {r4} %41: any
// CHKRA-NEXT:                 TryStartInst %BB13, %BB15
// CHKRA-NEXT:%BB12:
// CHKRA-NEXT:  {r1}     %47 = PhiInst (:any) {r1} %53: any, %BB13, {r1} %69: any, %BB18
// CHKRA-NEXT:  {r5}     %48 = PhiInst (:undefined|boolean) {r5} %54: undefined, %BB13, {r5} %70: boolean, %BB18
// CHKRA-NEXT:  {r1}     %49 = MovInst (:any) {r1} %47: any
// CHKRA-NEXT:  {r5}     %50 = MovInst (:undefined|boolean) {r5} %48: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {r5} %50: undefined|boolean, %BB65, %BB64
// CHKRA-NEXT:%BB13:
// CHKRA-NEXT:  {r6}     %52 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %53 = MovInst (:any) {r6} %52: any
// CHKRA-NEXT:  {r5}     %54 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB12
// CHKRA-NEXT:%BB14:
// CHKRA-NEXT:  {r1}     %56 = LoadStackInst (:any) {r1} %42: any
// CHKRA-NEXT:  {r1}     %57 = IteratorNextInst (:any) {r4} %41: any, {r1} %56: any
// CHKRA-NEXT:  {r5}     %58 = LoadStackInst (:any) {r4} %41: any
// CHKRA-NEXT:  {r5}     %59 = BinaryStrictlyEqualInst (:boolean) {r5} %58: any, {r0} %1: undefined
// CHKRA-NEXT:  {r7}     %60 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {r5} %59: boolean, %BB17, %BB16
// CHKRA-NEXT:%BB15:
// CHKRA-NEXT:                 TryEndInst %BB13, %BB14
// CHKRA-NEXT:%BB16:
// CHKRA-NEXT:  {r7}     %63 = MovInst (:any) {r1} %57: any
// CHKRA-NEXT:                 BranchInst %BB17
// CHKRA-NEXT:%BB17:
// CHKRA-NEXT:  {r7}     %65 = PhiInst (:any) {r7} %60: undefined, %BB14, {r7} %63: any, %BB16
// CHKRA-NEXT:  {r7}     %66 = MovInst (:any) {r7} %65: any
// CHKRA-NEXT:                 TryStartInst %BB18, %BB20
// CHKRA-NEXT:%BB18:
// CHKRA-NEXT:  {r1}     %68 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %69 = MovInst (:any) {r1} %68: any
// CHKRA-NEXT:  {r5}     %70 = MovInst (:boolean) {r5} %59: boolean
// CHKRA-NEXT:                 BranchInst %BB12
// CHKRA-NEXT:%BB19:
// CHKRA-NEXT:                 CondBranchInst {r5} %59: boolean, %BB63, %BB62
// CHKRA-NEXT:%BB20:
// CHKRA-NEXT:  {r6}     %73 = AllocStackInst (:any) $?anon_10_iter: any
// CHKRA-NEXT:  {r1}     %74 = AllocStackInst (:any) $?anon_11_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r7} %66: any, {r1} %74: any
// CHKRA-NEXT:  {r7}     %76 = IteratorBeginInst (:any) {r1} %74: any
// CHKRA-NEXT:                 StoreStackInst {r7} %76: any, {r6} %73: any
// CHKRA-NEXT:                 TryStartInst %BB22, %BB24
// CHKRA-NEXT:%BB21:
// CHKRA-NEXT:  {r1}     %79 = PhiInst (:any) {r1} %85: any, %BB22, {r1} %101: any, %BB27
// CHKRA-NEXT:  {r7}     %80 = PhiInst (:undefined|boolean) {r7} %86: undefined, %BB22, {r7} %102: boolean, %BB27
// CHKRA-NEXT:  {r1}     %81 = MovInst (:any) {r1} %79: any
// CHKRA-NEXT:  {r7}     %82 = MovInst (:undefined|boolean) {r7} %80: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {r7} %82: undefined|boolean, %BB61, %BB60
// CHKRA-NEXT:%BB22:
// CHKRA-NEXT:  {r8}     %84 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %85 = MovInst (:any) {r8} %84: any
// CHKRA-NEXT:  {r7}     %86 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB21
// CHKRA-NEXT:%BB23:
// CHKRA-NEXT:  {r1}     %88 = LoadStackInst (:any) {r1} %74: any
// CHKRA-NEXT:  {r1}     %89 = IteratorNextInst (:any) {r6} %73: any, {r1} %88: any
// CHKRA-NEXT:  {r7}     %90 = LoadStackInst (:any) {r6} %73: any
// CHKRA-NEXT:  {r7}     %91 = BinaryStrictlyEqualInst (:boolean) {r7} %90: any, {r0} %1: undefined
// CHKRA-NEXT:  {r9}     %92 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {r7} %91: boolean, %BB26, %BB25
// CHKRA-NEXT:%BB24:
// CHKRA-NEXT:                 TryEndInst %BB22, %BB23
// CHKRA-NEXT:%BB25:
// CHKRA-NEXT:  {r9}     %95 = MovInst (:any) {r1} %89: any
// CHKRA-NEXT:                 BranchInst %BB26
// CHKRA-NEXT:%BB26:
// CHKRA-NEXT:  {r9}     %97 = PhiInst (:any) {r9} %92: undefined, %BB23, {r9} %95: any, %BB25
// CHKRA-NEXT:  {r9}     %98 = MovInst (:any) {r9} %97: any
// CHKRA-NEXT:                 TryStartInst %BB27, %BB29
// CHKRA-NEXT:%BB27:
// CHKRA-NEXT:  {r1}     %100 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %101 = MovInst (:any) {r1} %100: any
// CHKRA-NEXT:  {r7}     %102 = MovInst (:boolean) {r7} %91: boolean
// CHKRA-NEXT:                 BranchInst %BB21
// CHKRA-NEXT:%BB28:
// CHKRA-NEXT:                 CondBranchInst {r7} %91: boolean, %BB59, %BB58
// CHKRA-NEXT:%BB29:
// CHKRA-NEXT:  {r8}     %105 = AllocStackInst (:any) $?anon_15_iter: any
// CHKRA-NEXT:  {r1}     %106 = AllocStackInst (:any) $?anon_16_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r9} %98: any, {r1} %106: any
// CHKRA-NEXT:  {r9}     %108 = IteratorBeginInst (:any) {r1} %106: any
// CHKRA-NEXT:                 StoreStackInst {r9} %108: any, {r8} %105: any
// CHKRA-NEXT:                 TryStartInst %BB31, %BB33
// CHKRA-NEXT:%BB30:
// CHKRA-NEXT:  {r1}     %111 = PhiInst (:any) {r1} %117: any, %BB31, {r1} %133: any, %BB36
// CHKRA-NEXT:  {r9}     %112 = PhiInst (:undefined|boolean) {r9} %118: undefined, %BB31, {r9} %134: boolean, %BB36
// CHKRA-NEXT:  {r1}     %113 = MovInst (:any) {r1} %111: any
// CHKRA-NEXT:  {r9}     %114 = MovInst (:undefined|boolean) {r9} %112: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {r9} %114: undefined|boolean, %BB57, %BB56
// CHKRA-NEXT:%BB31:
// CHKRA-NEXT:  {r10}    %116 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %117 = MovInst (:any) {r10} %116: any
// CHKRA-NEXT:  {r9}     %118 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB30
// CHKRA-NEXT:%BB32:
// CHKRA-NEXT:  {r1}     %120 = LoadStackInst (:any) {r1} %106: any
// CHKRA-NEXT:  {r10}    %121 = IteratorNextInst (:any) {r8} %105: any, {r1} %120: any
// CHKRA-NEXT:  {r1}     %122 = LoadStackInst (:any) {r8} %105: any
// CHKRA-NEXT:  {r9}     %123 = BinaryStrictlyEqualInst (:boolean) {r1} %122: any, {r0} %1: undefined
// CHKRA-NEXT:  {r1}     %124 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {r9} %123: boolean, %BB35, %BB34
// CHKRA-NEXT:%BB33:
// CHKRA-NEXT:                 TryEndInst %BB31, %BB32
// CHKRA-NEXT:%BB34:
// CHKRA-NEXT:  {r1}     %127 = MovInst (:any) {r10} %121: any
// CHKRA-NEXT:                 BranchInst %BB35
// CHKRA-NEXT:%BB35:
// CHKRA-NEXT:  {r1}     %129 = PhiInst (:any) {r1} %124: undefined, %BB32, {r1} %127: any, %BB34
// CHKRA-NEXT:  {r1}     %130 = MovInst (:any) {r1} %129: any
// CHKRA-NEXT:                 TryStartInst %BB36, %BB38
// CHKRA-NEXT:%BB36:
// CHKRA-NEXT:  {r1}     %132 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %133 = MovInst (:any) {r1} %132: any
// CHKRA-NEXT:  {r9}     %134 = MovInst (:boolean) {r9} %123: boolean
// CHKRA-NEXT:                 BranchInst %BB30
// CHKRA-NEXT:%BB37:
// CHKRA-NEXT:                 CondBranchInst {r9} %123: boolean, %BB55, %BB54
// CHKRA-NEXT:%BB38:
// CHKRA-NEXT:  {r11}    %137 = LoadPropertyInst (:any) {r1} %130: any, "z": string
// CHKRA-NEXT:  {r10}    %138 = AllocStackInst (:any) $?anon_20_iter: any
// CHKRA-NEXT:  {r1}     %139 = AllocStackInst (:any) $?anon_21_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r11} %137: any, {r1} %139: any
// CHKRA-NEXT:  {r11}    %141 = IteratorBeginInst (:any) {r1} %139: any
// CHKRA-NEXT:                 StoreStackInst {r11} %141: any, {r10} %138: any
// CHKRA-NEXT:                 TryStartInst %BB40, %BB42
// CHKRA-NEXT:%BB39:
// CHKRA-NEXT:  {r1}     %144 = PhiInst (:any) {r1} %150: any, %BB40, {r1} %166: any, %BB45
// CHKRA-NEXT:  {r11}    %145 = PhiInst (:undefined|boolean) {r11} %151: undefined, %BB40, {r11} %167: boolean, %BB45
// CHKRA-NEXT:  {r1}     %146 = MovInst (:any) {r1} %144: any
// CHKRA-NEXT:  {r11}    %147 = MovInst (:undefined|boolean) {r11} %145: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {r11} %147: undefined|boolean, %BB53, %BB52
// CHKRA-NEXT:%BB40:
// CHKRA-NEXT:  {r12}    %149 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %150 = MovInst (:any) {r12} %149: any
// CHKRA-NEXT:  {r11}    %151 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB39
// CHKRA-NEXT:%BB41:
// CHKRA-NEXT:  {r1}     %153 = LoadStackInst (:any) {r1} %139: any
// CHKRA-NEXT:  {r1}     %154 = IteratorNextInst (:any) {r10} %138: any, {r1} %153: any
// CHKRA-NEXT:  {r11}    %155 = LoadStackInst (:any) {r10} %138: any
// CHKRA-NEXT:  {r11}    %156 = BinaryStrictlyEqualInst (:boolean) {r11} %155: any, {r0} %1: undefined
// CHKRA-NEXT:  {r12}    %157 = MovInst (:undefined) {r0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {r11} %156: boolean, %BB44, %BB43
// CHKRA-NEXT:%BB42:
// CHKRA-NEXT:                 TryEndInst %BB40, %BB41
// CHKRA-NEXT:%BB43:
// CHKRA-NEXT:  {r12}    %160 = MovInst (:any) {r1} %154: any
// CHKRA-NEXT:                 BranchInst %BB44
// CHKRA-NEXT:%BB44:
// CHKRA-NEXT:  {r12}    %162 = PhiInst (:any) {r12} %157: undefined, %BB41, {r12} %160: any, %BB43
// CHKRA-NEXT:  {r12}    %163 = MovInst (:any) {r12} %162: any
// CHKRA-NEXT:                 TryStartInst %BB45, %BB47
// CHKRA-NEXT:%BB45:
// CHKRA-NEXT:  {r1}     %165 = CatchInst (:any)
// CHKRA-NEXT:  {r1}     %166 = MovInst (:any) {r1} %165: any
// CHKRA-NEXT:  {r11}    %167 = MovInst (:boolean) {r11} %156: boolean
// CHKRA-NEXT:                 BranchInst %BB39
// CHKRA-NEXT:%BB46:
// CHKRA-NEXT:                 CondBranchInst {r11} %156: boolean, %BB51, %BB50
// CHKRA-NEXT:%BB47:
// CHKRA-NEXT:  {r1}     %170 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:                 CmpBrEqualInst {r12} %163: any, {r1} %170: null, %BB48, %BB49
// CHKRA-NEXT:%BB48:
// CHKRA-NEXT:  {r1}     %172 = HBCLoadConstInst (:string) "Cannot destructure 'undefined' or 'null'.": string
// CHKRA-NEXT:  {r1}     %173 = CallBuiltinInst (:any) [HermesBuiltin.throwTypeError]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, {r1} %172: string
// CHKRA-NEXT:                 UnreachableInst
// CHKRA-NEXT:%BB49:
// CHKRA-NEXT:                 TryEndInst %BB45, %BB46
// CHKRA-NEXT:%BB50:
// CHKRA-NEXT:  {r1}     %176 = LoadStackInst (:any) {r10} %138: any
// CHKRA-NEXT:  {r1}     %177 = IteratorCloseInst (:any) {r1} %176: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB51
// CHKRA-NEXT:%BB51:
// CHKRA-NEXT:                 TryEndInst %BB36, %BB37
// CHKRA-NEXT:%BB52:
// CHKRA-NEXT:  {r10}    %180 = LoadStackInst (:any) {r10} %138: any
// CHKRA-NEXT:  {r10}    %181 = IteratorCloseInst (:any) {r10} %180: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB53
// CHKRA-NEXT:%BB53:
// CHKRA-NEXT:                 ThrowInst {r1} %146: any, %BB36
// CHKRA-NEXT:%BB54:
// CHKRA-NEXT:  {r1}     %184 = LoadStackInst (:any) {r8} %105: any
// CHKRA-NEXT:  {r1}     %185 = IteratorCloseInst (:any) {r1} %184: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB55
// CHKRA-NEXT:%BB55:
// CHKRA-NEXT:                 TryEndInst %BB27, %BB28
// CHKRA-NEXT:%BB56:
// CHKRA-NEXT:  {r8}     %188 = LoadStackInst (:any) {r8} %105: any
// CHKRA-NEXT:  {r8}     %189 = IteratorCloseInst (:any) {r8} %188: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB57
// CHKRA-NEXT:%BB57:
// CHKRA-NEXT:                 ThrowInst {r1} %113: any, %BB27
// CHKRA-NEXT:%BB58:
// CHKRA-NEXT:  {r1}     %192 = LoadStackInst (:any) {r6} %73: any
// CHKRA-NEXT:  {r1}     %193 = IteratorCloseInst (:any) {r1} %192: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB59
// CHKRA-NEXT:%BB59:
// CHKRA-NEXT:                 TryEndInst %BB18, %BB19
// CHKRA-NEXT:%BB60:
// CHKRA-NEXT:  {r6}     %196 = LoadStackInst (:any) {r6} %73: any
// CHKRA-NEXT:  {r6}     %197 = IteratorCloseInst (:any) {r6} %196: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB61
// CHKRA-NEXT:%BB61:
// CHKRA-NEXT:                 ThrowInst {r1} %81: any, %BB18
// CHKRA-NEXT:%BB62:
// CHKRA-NEXT:  {r1}     %200 = LoadStackInst (:any) {r4} %41: any
// CHKRA-NEXT:  {r1}     %201 = IteratorCloseInst (:any) {r1} %200: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB63
// CHKRA-NEXT:%BB63:
// CHKRA-NEXT:                 TryEndInst %BB9, %BB10
// CHKRA-NEXT:%BB64:
// CHKRA-NEXT:  {r4}     %204 = LoadStackInst (:any) {r4} %41: any
// CHKRA-NEXT:  {r4}     %205 = IteratorCloseInst (:any) {r4} %204: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB65
// CHKRA-NEXT:%BB65:
// CHKRA-NEXT:                 ThrowInst {r1} %49: any, %BB9
// CHKRA-NEXT:%BB66:
// CHKRA-NEXT:  {r1}     %208 = LoadStackInst (:any) {r2} %5: any
// CHKRA-NEXT:  {r1}     %209 = IteratorCloseInst (:any) {r1} %208: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB2
// CHKRA-NEXT:%BB67:
// CHKRA-NEXT:  {r2}     %211 = LoadStackInst (:any) {r2} %5: any
// CHKRA-NEXT:  {r2}     %212 = IteratorCloseInst (:any) {r2} %211: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB68
// CHKRA-NEXT:%BB68:
// CHKRA-NEXT:                 ThrowInst {r1} %17: any
// CHKRA-NEXT:function_end

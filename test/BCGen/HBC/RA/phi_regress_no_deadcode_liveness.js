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
// CHKRA-NEXT:  {r2}      %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKRA-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:                 StoreStackInst {np0} %1: undefined, {r2} %0: undefined|object
// CHKRA-NEXT:  {r0}      %3 = LoadParamInst (:any) %d: any
// CHKRA-NEXT:                 CmpBrStrictlyNotEqualInst {r0} %3: any, {np0} %1: undefined, %BB2, %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  {r1}      %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHKRA-NEXT:  {r0}      %6 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHKRA-NEXT:                 HBCReifyArgumentsLooseInst {r2} %0: undefined|object
// CHKRA-NEXT:  {r2}      %8 = LoadStackInst (:undefined|object) {r2} %0: undefined|object
// CHKRA-NEXT:  {r2}      %9 = UnionNarrowTrustedInst (:object) {r2} %8: undefined|object
// CHKRA-NEXT:                 StoreStackInst {r2} %9: object, {r0} %6: any
// CHKRA-NEXT:  {r2}     %11 = IteratorBeginInst (:any) {r0} %6: any
// CHKRA-NEXT:                 StoreStackInst {r2} %11: any, {r1} %5: any
// CHKRA-NEXT:                 TryStartInst %BB4, %BB6
// CHKRA-NEXT:%BB2:
// CHKRA-NEXT:                 ReturnInst {np0} %1: undefined
// CHKRA-NEXT:%BB3:
// CHKRA-NEXT:  {r0}     %15 = PhiInst (:any) {r0} %21: any, %BB4, {r0} %37: any, %BB9
// CHKRA-NEXT:  {np1}    %16 = PhiInst (:undefined|boolean) {np1} %22: undefined, %BB4, {np1} %38: boolean, %BB9
// CHKRA-NEXT:  {r0}     %17 = MovInst (:any) {r0} %15: any
// CHKRA-NEXT:  {np1}    %18 = MovInst (:undefined|boolean) {np1} %16: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {np1} %18: undefined|boolean, %BB68, %BB67
// CHKRA-NEXT:%BB4:
// CHKRA-NEXT:  {r2}     %20 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %21 = MovInst (:any) {r2} %20: any
// CHKRA-NEXT:  {np1}    %22 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB3
// CHKRA-NEXT:%BB5:
// CHKRA-NEXT:  {r0}     %24 = LoadStackInst (:any) {r0} %6: any
// CHKRA-NEXT:  {r0}     %25 = IteratorNextInst (:any) {r1} %5: any, {r0} %24: any
// CHKRA-NEXT:  {r2}     %26 = LoadStackInst (:any) {r1} %5: any
// CHKRA-NEXT:  {np1}    %27 = BinaryStrictlyEqualInst (:boolean) {r2} %26: any, {np0} %1: undefined
// CHKRA-NEXT:  {r3}     %28 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {np1} %27: boolean, %BB8, %BB7
// CHKRA-NEXT:%BB6:
// CHKRA-NEXT:                 TryEndInst %BB4, %BB5
// CHKRA-NEXT:%BB7:
// CHKRA-NEXT:  {r3}     %31 = MovInst (:any) {r0} %25: any
// CHKRA-NEXT:                 BranchInst %BB8
// CHKRA-NEXT:%BB8:
// CHKRA-NEXT:  {r3}     %33 = PhiInst (:any) {r3} %28: undefined, %BB5, {r3} %31: any, %BB7
// CHKRA-NEXT:  {r3}     %34 = MovInst (:any) {r3} %33: any
// CHKRA-NEXT:                 TryStartInst %BB9, %BB11
// CHKRA-NEXT:%BB9:
// CHKRA-NEXT:  {r0}     %36 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %37 = MovInst (:any) {r0} %36: any
// CHKRA-NEXT:  {np1}    %38 = MovInst (:boolean) {np1} %27: boolean
// CHKRA-NEXT:                 BranchInst %BB3
// CHKRA-NEXT:%BB10:
// CHKRA-NEXT:                 CondBranchInst {np1} %27: boolean, %BB2, %BB66
// CHKRA-NEXT:%BB11:
// CHKRA-NEXT:  {r2}     %41 = AllocStackInst (:any) $?anon_5_iter: any
// CHKRA-NEXT:  {r0}     %42 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r3} %34: any, {r0} %42: any
// CHKRA-NEXT:  {r3}     %44 = IteratorBeginInst (:any) {r0} %42: any
// CHKRA-NEXT:                 StoreStackInst {r3} %44: any, {r2} %41: any
// CHKRA-NEXT:                 TryStartInst %BB13, %BB15
// CHKRA-NEXT:%BB12:
// CHKRA-NEXT:  {r0}     %47 = PhiInst (:any) {r0} %53: any, %BB13, {r0} %69: any, %BB18
// CHKRA-NEXT:  {np2}    %48 = PhiInst (:undefined|boolean) {np2} %54: undefined, %BB13, {np2} %70: boolean, %BB18
// CHKRA-NEXT:  {r0}     %49 = MovInst (:any) {r0} %47: any
// CHKRA-NEXT:  {np2}    %50 = MovInst (:undefined|boolean) {np2} %48: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {np2} %50: undefined|boolean, %BB65, %BB64
// CHKRA-NEXT:%BB13:
// CHKRA-NEXT:  {r3}     %52 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %53 = MovInst (:any) {r3} %52: any
// CHKRA-NEXT:  {np2}    %54 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB12
// CHKRA-NEXT:%BB14:
// CHKRA-NEXT:  {r0}     %56 = LoadStackInst (:any) {r0} %42: any
// CHKRA-NEXT:  {r0}     %57 = IteratorNextInst (:any) {r2} %41: any, {r0} %56: any
// CHKRA-NEXT:  {r3}     %58 = LoadStackInst (:any) {r2} %41: any
// CHKRA-NEXT:  {np2}    %59 = BinaryStrictlyEqualInst (:boolean) {r3} %58: any, {np0} %1: undefined
// CHKRA-NEXT:  {r4}     %60 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {np2} %59: boolean, %BB17, %BB16
// CHKRA-NEXT:%BB15:
// CHKRA-NEXT:                 TryEndInst %BB13, %BB14
// CHKRA-NEXT:%BB16:
// CHKRA-NEXT:  {r4}     %63 = MovInst (:any) {r0} %57: any
// CHKRA-NEXT:                 BranchInst %BB17
// CHKRA-NEXT:%BB17:
// CHKRA-NEXT:  {r4}     %65 = PhiInst (:any) {r4} %60: undefined, %BB14, {r4} %63: any, %BB16
// CHKRA-NEXT:  {r4}     %66 = MovInst (:any) {r4} %65: any
// CHKRA-NEXT:                 TryStartInst %BB18, %BB20
// CHKRA-NEXT:%BB18:
// CHKRA-NEXT:  {r0}     %68 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %69 = MovInst (:any) {r0} %68: any
// CHKRA-NEXT:  {np2}    %70 = MovInst (:boolean) {np2} %59: boolean
// CHKRA-NEXT:                 BranchInst %BB12
// CHKRA-NEXT:%BB19:
// CHKRA-NEXT:                 CondBranchInst {np2} %59: boolean, %BB63, %BB62
// CHKRA-NEXT:%BB20:
// CHKRA-NEXT:  {r3}     %73 = AllocStackInst (:any) $?anon_10_iter: any
// CHKRA-NEXT:  {r0}     %74 = AllocStackInst (:any) $?anon_11_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r4} %66: any, {r0} %74: any
// CHKRA-NEXT:  {r4}     %76 = IteratorBeginInst (:any) {r0} %74: any
// CHKRA-NEXT:                 StoreStackInst {r4} %76: any, {r3} %73: any
// CHKRA-NEXT:                 TryStartInst %BB22, %BB24
// CHKRA-NEXT:%BB21:
// CHKRA-NEXT:  {r0}     %79 = PhiInst (:any) {r0} %85: any, %BB22, {r0} %101: any, %BB27
// CHKRA-NEXT:  {np3}    %80 = PhiInst (:undefined|boolean) {np3} %86: undefined, %BB22, {np3} %102: boolean, %BB27
// CHKRA-NEXT:  {r0}     %81 = MovInst (:any) {r0} %79: any
// CHKRA-NEXT:  {np3}    %82 = MovInst (:undefined|boolean) {np3} %80: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {np3} %82: undefined|boolean, %BB61, %BB60
// CHKRA-NEXT:%BB22:
// CHKRA-NEXT:  {r4}     %84 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %85 = MovInst (:any) {r4} %84: any
// CHKRA-NEXT:  {np3}    %86 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB21
// CHKRA-NEXT:%BB23:
// CHKRA-NEXT:  {r0}     %88 = LoadStackInst (:any) {r0} %74: any
// CHKRA-NEXT:  {r0}     %89 = IteratorNextInst (:any) {r3} %73: any, {r0} %88: any
// CHKRA-NEXT:  {r4}     %90 = LoadStackInst (:any) {r3} %73: any
// CHKRA-NEXT:  {np3}    %91 = BinaryStrictlyEqualInst (:boolean) {r4} %90: any, {np0} %1: undefined
// CHKRA-NEXT:  {r5}     %92 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {np3} %91: boolean, %BB26, %BB25
// CHKRA-NEXT:%BB24:
// CHKRA-NEXT:                 TryEndInst %BB22, %BB23
// CHKRA-NEXT:%BB25:
// CHKRA-NEXT:  {r5}     %95 = MovInst (:any) {r0} %89: any
// CHKRA-NEXT:                 BranchInst %BB26
// CHKRA-NEXT:%BB26:
// CHKRA-NEXT:  {r5}     %97 = PhiInst (:any) {r5} %92: undefined, %BB23, {r5} %95: any, %BB25
// CHKRA-NEXT:  {r5}     %98 = MovInst (:any) {r5} %97: any
// CHKRA-NEXT:                 TryStartInst %BB27, %BB29
// CHKRA-NEXT:%BB27:
// CHKRA-NEXT:  {r0}     %100 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %101 = MovInst (:any) {r0} %100: any
// CHKRA-NEXT:  {np3}    %102 = MovInst (:boolean) {np3} %91: boolean
// CHKRA-NEXT:                 BranchInst %BB21
// CHKRA-NEXT:%BB28:
// CHKRA-NEXT:                 CondBranchInst {np3} %91: boolean, %BB59, %BB58
// CHKRA-NEXT:%BB29:
// CHKRA-NEXT:  {r4}     %105 = AllocStackInst (:any) $?anon_15_iter: any
// CHKRA-NEXT:  {r0}     %106 = AllocStackInst (:any) $?anon_16_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r5} %98: any, {r0} %106: any
// CHKRA-NEXT:  {r5}     %108 = IteratorBeginInst (:any) {r0} %106: any
// CHKRA-NEXT:                 StoreStackInst {r5} %108: any, {r4} %105: any
// CHKRA-NEXT:                 TryStartInst %BB31, %BB33
// CHKRA-NEXT:%BB30:
// CHKRA-NEXT:  {r0}     %111 = PhiInst (:any) {r0} %117: any, %BB31, {r0} %133: any, %BB36
// CHKRA-NEXT:  {np4}    %112 = PhiInst (:undefined|boolean) {np4} %118: undefined, %BB31, {np4} %134: boolean, %BB36
// CHKRA-NEXT:  {r0}     %113 = MovInst (:any) {r0} %111: any
// CHKRA-NEXT:  {np4}    %114 = MovInst (:undefined|boolean) {np4} %112: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {np4} %114: undefined|boolean, %BB57, %BB56
// CHKRA-NEXT:%BB31:
// CHKRA-NEXT:  {r5}     %116 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %117 = MovInst (:any) {r5} %116: any
// CHKRA-NEXT:  {np4}    %118 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB30
// CHKRA-NEXT:%BB32:
// CHKRA-NEXT:  {r0}     %120 = LoadStackInst (:any) {r0} %106: any
// CHKRA-NEXT:  {r5}     %121 = IteratorNextInst (:any) {r4} %105: any, {r0} %120: any
// CHKRA-NEXT:  {r0}     %122 = LoadStackInst (:any) {r4} %105: any
// CHKRA-NEXT:  {np4}    %123 = BinaryStrictlyEqualInst (:boolean) {r0} %122: any, {np0} %1: undefined
// CHKRA-NEXT:  {r0}     %124 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {np4} %123: boolean, %BB35, %BB34
// CHKRA-NEXT:%BB33:
// CHKRA-NEXT:                 TryEndInst %BB31, %BB32
// CHKRA-NEXT:%BB34:
// CHKRA-NEXT:  {r0}     %127 = MovInst (:any) {r5} %121: any
// CHKRA-NEXT:                 BranchInst %BB35
// CHKRA-NEXT:%BB35:
// CHKRA-NEXT:  {r0}     %129 = PhiInst (:any) {r0} %124: undefined, %BB32, {r0} %127: any, %BB34
// CHKRA-NEXT:  {r0}     %130 = MovInst (:any) {r0} %129: any
// CHKRA-NEXT:                 TryStartInst %BB36, %BB38
// CHKRA-NEXT:%BB36:
// CHKRA-NEXT:  {r0}     %132 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %133 = MovInst (:any) {r0} %132: any
// CHKRA-NEXT:  {np4}    %134 = MovInst (:boolean) {np4} %123: boolean
// CHKRA-NEXT:                 BranchInst %BB30
// CHKRA-NEXT:%BB37:
// CHKRA-NEXT:                 CondBranchInst {np4} %123: boolean, %BB55, %BB54
// CHKRA-NEXT:%BB38:
// CHKRA-NEXT:  {r6}     %137 = LoadPropertyInst (:any) {r0} %130: any, "z": string
// CHKRA-NEXT:  {r5}     %138 = AllocStackInst (:any) $?anon_20_iter: any
// CHKRA-NEXT:  {r0}     %139 = AllocStackInst (:any) $?anon_21_sourceOrNext: any
// CHKRA-NEXT:                 StoreStackInst {r6} %137: any, {r0} %139: any
// CHKRA-NEXT:  {r6}     %141 = IteratorBeginInst (:any) {r0} %139: any
// CHKRA-NEXT:                 StoreStackInst {r6} %141: any, {r5} %138: any
// CHKRA-NEXT:                 TryStartInst %BB40, %BB42
// CHKRA-NEXT:%BB39:
// CHKRA-NEXT:  {r0}     %144 = PhiInst (:any) {r0} %150: any, %BB40, {r0} %166: any, %BB45
// CHKRA-NEXT:  {np5}    %145 = PhiInst (:undefined|boolean) {np5} %151: undefined, %BB40, {np5} %167: boolean, %BB45
// CHKRA-NEXT:  {r0}     %146 = MovInst (:any) {r0} %144: any
// CHKRA-NEXT:  {np5}    %147 = MovInst (:undefined|boolean) {np5} %145: undefined|boolean
// CHKRA-NEXT:                 CondBranchInst {np5} %147: undefined|boolean, %BB53, %BB52
// CHKRA-NEXT:%BB40:
// CHKRA-NEXT:  {r6}     %149 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %150 = MovInst (:any) {r6} %149: any
// CHKRA-NEXT:  {np5}    %151 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 BranchInst %BB39
// CHKRA-NEXT:%BB41:
// CHKRA-NEXT:  {r0}     %153 = LoadStackInst (:any) {r0} %139: any
// CHKRA-NEXT:  {r6}     %154 = IteratorNextInst (:any) {r5} %138: any, {r0} %153: any
// CHKRA-NEXT:  {r0}     %155 = LoadStackInst (:any) {r5} %138: any
// CHKRA-NEXT:  {np5}    %156 = BinaryStrictlyEqualInst (:boolean) {r0} %155: any, {np0} %1: undefined
// CHKRA-NEXT:  {r0}     %157 = MovInst (:undefined) {np0} %1: undefined
// CHKRA-NEXT:                 CondBranchInst {np5} %156: boolean, %BB44, %BB43
// CHKRA-NEXT:%BB42:
// CHKRA-NEXT:                 TryEndInst %BB40, %BB41
// CHKRA-NEXT:%BB43:
// CHKRA-NEXT:  {r0}     %160 = MovInst (:any) {r6} %154: any
// CHKRA-NEXT:                 BranchInst %BB44
// CHKRA-NEXT:%BB44:
// CHKRA-NEXT:  {r0}     %162 = PhiInst (:any) {r0} %157: undefined, %BB41, {r0} %160: any, %BB43
// CHKRA-NEXT:  {r0}     %163 = MovInst (:any) {r0} %162: any
// CHKRA-NEXT:                 TryStartInst %BB45, %BB47
// CHKRA-NEXT:%BB45:
// CHKRA-NEXT:  {r0}     %165 = CatchInst (:any)
// CHKRA-NEXT:  {r0}     %166 = MovInst (:any) {r0} %165: any
// CHKRA-NEXT:  {np5}    %167 = MovInst (:boolean) {np5} %156: boolean
// CHKRA-NEXT:                 BranchInst %BB39
// CHKRA-NEXT:%BB46:
// CHKRA-NEXT:                 CondBranchInst {np5} %156: boolean, %BB51, %BB50
// CHKRA-NEXT:%BB47:
// CHKRA-NEXT:  {np6}    %170 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:                 CmpBrEqualInst {r0} %163: any, {np6} %170: null, %BB48, %BB49
// CHKRA-NEXT:%BB48:
// CHKRA-NEXT:  {r0}     %172 = HBCLoadConstInst (:string) "Cannot destructure 'undefined' or 'null'.": string
// CHKRA-NEXT:  {r0}     %173 = CallBuiltinInst (:any) [HermesBuiltin.throwTypeError]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, {r0} %172: string
// CHKRA-NEXT:                 UnreachableInst
// CHKRA-NEXT:%BB49:
// CHKRA-NEXT:                 TryEndInst %BB45, %BB46
// CHKRA-NEXT:%BB50:
// CHKRA-NEXT:  {r0}     %176 = LoadStackInst (:any) {r5} %138: any
// CHKRA-NEXT:  {r0}     %177 = IteratorCloseInst (:any) {r0} %176: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB51
// CHKRA-NEXT:%BB51:
// CHKRA-NEXT:                 TryEndInst %BB36, %BB37
// CHKRA-NEXT:%BB52:
// CHKRA-NEXT:  {r5}     %180 = LoadStackInst (:any) {r5} %138: any
// CHKRA-NEXT:  {r5}     %181 = IteratorCloseInst (:any) {r5} %180: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB53
// CHKRA-NEXT:%BB53:
// CHKRA-NEXT:                 ThrowInst {r0} %146: any, %BB36
// CHKRA-NEXT:%BB54:
// CHKRA-NEXT:  {r0}     %184 = LoadStackInst (:any) {r4} %105: any
// CHKRA-NEXT:  {r0}     %185 = IteratorCloseInst (:any) {r0} %184: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB55
// CHKRA-NEXT:%BB55:
// CHKRA-NEXT:                 TryEndInst %BB27, %BB28
// CHKRA-NEXT:%BB56:
// CHKRA-NEXT:  {r4}     %188 = LoadStackInst (:any) {r4} %105: any
// CHKRA-NEXT:  {r4}     %189 = IteratorCloseInst (:any) {r4} %188: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB57
// CHKRA-NEXT:%BB57:
// CHKRA-NEXT:                 ThrowInst {r0} %113: any, %BB27
// CHKRA-NEXT:%BB58:
// CHKRA-NEXT:  {r0}     %192 = LoadStackInst (:any) {r3} %73: any
// CHKRA-NEXT:  {r0}     %193 = IteratorCloseInst (:any) {r0} %192: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB59
// CHKRA-NEXT:%BB59:
// CHKRA-NEXT:                 TryEndInst %BB18, %BB19
// CHKRA-NEXT:%BB60:
// CHKRA-NEXT:  {r3}     %196 = LoadStackInst (:any) {r3} %73: any
// CHKRA-NEXT:  {r3}     %197 = IteratorCloseInst (:any) {r3} %196: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB61
// CHKRA-NEXT:%BB61:
// CHKRA-NEXT:                 ThrowInst {r0} %81: any, %BB18
// CHKRA-NEXT:%BB62:
// CHKRA-NEXT:  {r0}     %200 = LoadStackInst (:any) {r2} %41: any
// CHKRA-NEXT:  {r0}     %201 = IteratorCloseInst (:any) {r0} %200: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB63
// CHKRA-NEXT:%BB63:
// CHKRA-NEXT:                 TryEndInst %BB9, %BB10
// CHKRA-NEXT:%BB64:
// CHKRA-NEXT:  {r2}     %204 = LoadStackInst (:any) {r2} %41: any
// CHKRA-NEXT:  {r2}     %205 = IteratorCloseInst (:any) {r2} %204: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB65
// CHKRA-NEXT:%BB65:
// CHKRA-NEXT:                 ThrowInst {r0} %49: any, %BB9
// CHKRA-NEXT:%BB66:
// CHKRA-NEXT:  {r0}     %208 = LoadStackInst (:any) {r1} %5: any
// CHKRA-NEXT:  {r0}     %209 = IteratorCloseInst (:any) {r0} %208: any, false: boolean
// CHKRA-NEXT:                 BranchInst %BB2
// CHKRA-NEXT:%BB67:
// CHKRA-NEXT:  {r1}     %211 = LoadStackInst (:any) {r1} %5: any
// CHKRA-NEXT:  {r1}     %212 = IteratorCloseInst (:any) {r1} %211: any, true: boolean
// CHKRA-NEXT:                 BranchInst %BB68
// CHKRA-NEXT:%BB68:
// CHKRA-NEXT:                 ThrowInst {r0} %17: any
// CHKRA-NEXT:function_end

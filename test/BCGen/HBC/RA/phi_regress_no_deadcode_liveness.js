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
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "b": string
// CHKRA-NEXT:  $Reg0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKRA-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %b(): functionCode
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "b": string
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function b(d: any): undefined
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg3 = AllocStackInst (:undefined|object) $arguments: any
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg1 = StoreStackInst $Reg0, $Reg3
// CHKRA-NEXT:  $Reg1 = LoadParamInst (:any) %d: any
// CHKRA-NEXT:  $Reg1 = CmpBrStrictlyNotEqualInst $Reg1, $Reg0, %BB2, %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  $Reg2 = AllocStackInst (:any) $?anon_0_iter: any
// CHKRA-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHKRA-NEXT:  $Reg4 = HBCReifyArgumentsLooseInst $Reg3
// CHKRA-NEXT:  $Reg3 = LoadStackInst (:undefined|object) $Reg3
// CHKRA-NEXT:  $Reg3 = UnionNarrowTrustedInst (:object) $Reg3
// CHKRA-NEXT:  $Reg3 = StoreStackInst $Reg3, $Reg1
// CHKRA-NEXT:  $Reg3 = IteratorBeginInst (:any) $Reg1
// CHKRA-NEXT:  $Reg3 = StoreStackInst $Reg3, $Reg2
// CHKRA-NEXT:  $Reg3 = TryStartInst %BB4, %BB6
// CHKRA-NEXT:%BB2:
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:%BB3:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB4, $Reg1, %BB9
// CHKRA-NEXT:  $Reg3 = PhiInst (:undefined|boolean) $Reg3, %BB4, $Reg3, %BB9
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg3 = MovInst (:undefined|boolean) $Reg3
// CHKRA-NEXT:  $Reg3 = CondBranchInst $Reg3, %BB68, %BB67
// CHKRA-NEXT:%BB4:
// CHKRA-NEXT:  $Reg4 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg4
// CHKRA-NEXT:  $Reg3 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg4 = BranchInst %BB3
// CHKRA-NEXT:%BB5:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = IteratorNextInst (:any) $Reg2, $Reg1
// CHKRA-NEXT:  $Reg3 = LoadStackInst (:any) $Reg2
// CHKRA-NEXT:  $Reg3 = BinaryStrictlyEqualInst (:boolean) $Reg3, $Reg0
// CHKRA-NEXT:  $Reg5 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg4 = CondBranchInst $Reg3, %BB8, %BB7
// CHKRA-NEXT:%BB6:
// CHKRA-NEXT:  $Reg3 = TryEndInst %BB4, %BB5
// CHKRA-NEXT:%BB7:
// CHKRA-NEXT:  $Reg5 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = BranchInst %BB8
// CHKRA-NEXT:%BB8:
// CHKRA-NEXT:  $Reg5 = PhiInst (:any) $Reg5, %BB5, $Reg5, %BB7
// CHKRA-NEXT:  $Reg5 = MovInst (:any) $Reg5
// CHKRA-NEXT:  $Reg1 = TryStartInst %BB9, %BB11
// CHKRA-NEXT:%BB9:
// CHKRA-NEXT:  $Reg1 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg3 = MovInst (:boolean) $Reg3
// CHKRA-NEXT:  $Reg4 = BranchInst %BB3
// CHKRA-NEXT:%BB10:
// CHKRA-NEXT:  $Reg1 = CondBranchInst $Reg3, %BB2, %BB66
// CHKRA-NEXT:%BB11:
// CHKRA-NEXT:  $Reg4 = AllocStackInst (:any) $?anon_5_iter: any
// CHKRA-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHKRA-NEXT:  $Reg5 = StoreStackInst $Reg5, $Reg1
// CHKRA-NEXT:  $Reg5 = IteratorBeginInst (:any) $Reg1
// CHKRA-NEXT:  $Reg5 = StoreStackInst $Reg5, $Reg4
// CHKRA-NEXT:  $Reg5 = TryStartInst %BB13, %BB15
// CHKRA-NEXT:%BB12:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB13, $Reg1, %BB18
// CHKRA-NEXT:  $Reg5 = PhiInst (:undefined|boolean) $Reg5, %BB13, $Reg5, %BB18
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg5 = MovInst (:undefined|boolean) $Reg5
// CHKRA-NEXT:  $Reg5 = CondBranchInst $Reg5, %BB65, %BB64
// CHKRA-NEXT:%BB13:
// CHKRA-NEXT:  $Reg6 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg6
// CHKRA-NEXT:  $Reg5 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg6 = BranchInst %BB12
// CHKRA-NEXT:%BB14:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = IteratorNextInst (:any) $Reg4, $Reg1
// CHKRA-NEXT:  $Reg5 = LoadStackInst (:any) $Reg4
// CHKRA-NEXT:  $Reg5 = BinaryStrictlyEqualInst (:boolean) $Reg5, $Reg0
// CHKRA-NEXT:  $Reg7 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg6 = CondBranchInst $Reg5, %BB17, %BB16
// CHKRA-NEXT:%BB15:
// CHKRA-NEXT:  $Reg5 = TryEndInst %BB13, %BB14
// CHKRA-NEXT:%BB16:
// CHKRA-NEXT:  $Reg7 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = BranchInst %BB17
// CHKRA-NEXT:%BB17:
// CHKRA-NEXT:  $Reg7 = PhiInst (:any) $Reg7, %BB14, $Reg7, %BB16
// CHKRA-NEXT:  $Reg7 = MovInst (:any) $Reg7
// CHKRA-NEXT:  $Reg1 = TryStartInst %BB18, %BB20
// CHKRA-NEXT:%BB18:
// CHKRA-NEXT:  $Reg1 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg5 = MovInst (:boolean) $Reg5
// CHKRA-NEXT:  $Reg6 = BranchInst %BB12
// CHKRA-NEXT:%BB19:
// CHKRA-NEXT:  $Reg1 = CondBranchInst $Reg5, %BB63, %BB62
// CHKRA-NEXT:%BB20:
// CHKRA-NEXT:  $Reg6 = AllocStackInst (:any) $?anon_10_iter: any
// CHKRA-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_11_sourceOrNext: any
// CHKRA-NEXT:  $Reg7 = StoreStackInst $Reg7, $Reg1
// CHKRA-NEXT:  $Reg7 = IteratorBeginInst (:any) $Reg1
// CHKRA-NEXT:  $Reg7 = StoreStackInst $Reg7, $Reg6
// CHKRA-NEXT:  $Reg7 = TryStartInst %BB22, %BB24
// CHKRA-NEXT:%BB21:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB22, $Reg1, %BB27
// CHKRA-NEXT:  $Reg7 = PhiInst (:undefined|boolean) $Reg7, %BB22, $Reg7, %BB27
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg7 = MovInst (:undefined|boolean) $Reg7
// CHKRA-NEXT:  $Reg7 = CondBranchInst $Reg7, %BB61, %BB60
// CHKRA-NEXT:%BB22:
// CHKRA-NEXT:  $Reg8 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg8
// CHKRA-NEXT:  $Reg7 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg8 = BranchInst %BB21
// CHKRA-NEXT:%BB23:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = IteratorNextInst (:any) $Reg6, $Reg1
// CHKRA-NEXT:  $Reg7 = LoadStackInst (:any) $Reg6
// CHKRA-NEXT:  $Reg7 = BinaryStrictlyEqualInst (:boolean) $Reg7, $Reg0
// CHKRA-NEXT:  $Reg9 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg8 = CondBranchInst $Reg7, %BB26, %BB25
// CHKRA-NEXT:%BB24:
// CHKRA-NEXT:  $Reg7 = TryEndInst %BB22, %BB23
// CHKRA-NEXT:%BB25:
// CHKRA-NEXT:  $Reg9 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = BranchInst %BB26
// CHKRA-NEXT:%BB26:
// CHKRA-NEXT:  $Reg9 = PhiInst (:any) $Reg9, %BB23, $Reg9, %BB25
// CHKRA-NEXT:  $Reg9 = MovInst (:any) $Reg9
// CHKRA-NEXT:  $Reg1 = TryStartInst %BB27, %BB29
// CHKRA-NEXT:%BB27:
// CHKRA-NEXT:  $Reg1 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg7 = MovInst (:boolean) $Reg7
// CHKRA-NEXT:  $Reg8 = BranchInst %BB21
// CHKRA-NEXT:%BB28:
// CHKRA-NEXT:  $Reg1 = CondBranchInst $Reg7, %BB59, %BB58
// CHKRA-NEXT:%BB29:
// CHKRA-NEXT:  $Reg8 = AllocStackInst (:any) $?anon_15_iter: any
// CHKRA-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_16_sourceOrNext: any
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg9, $Reg1
// CHKRA-NEXT:  $Reg9 = IteratorBeginInst (:any) $Reg1
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg9, $Reg8
// CHKRA-NEXT:  $Reg9 = TryStartInst %BB31, %BB33
// CHKRA-NEXT:%BB30:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB31, $Reg1, %BB36
// CHKRA-NEXT:  $Reg9 = PhiInst (:undefined|boolean) $Reg9, %BB31, $Reg9, %BB36
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg9 = MovInst (:undefined|boolean) $Reg9
// CHKRA-NEXT:  $Reg9 = CondBranchInst $Reg9, %BB57, %BB56
// CHKRA-NEXT:%BB31:
// CHKRA-NEXT:  $Reg10 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg10
// CHKRA-NEXT:  $Reg9 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg10 = BranchInst %BB30
// CHKRA-NEXT:%BB32:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHKRA-NEXT:  $Reg10 = IteratorNextInst (:any) $Reg8, $Reg1
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg8
// CHKRA-NEXT:  $Reg9 = BinaryStrictlyEqualInst (:boolean) $Reg1, $Reg0
// CHKRA-NEXT:  $Reg1 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg11 = CondBranchInst $Reg9, %BB35, %BB34
// CHKRA-NEXT:%BB33:
// CHKRA-NEXT:  $Reg9 = TryEndInst %BB31, %BB32
// CHKRA-NEXT:%BB34:
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg10
// CHKRA-NEXT:  $Reg10 = BranchInst %BB35
// CHKRA-NEXT:%BB35:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB32, $Reg1, %BB34
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg10 = TryStartInst %BB36, %BB38
// CHKRA-NEXT:%BB36:
// CHKRA-NEXT:  $Reg1 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg9 = MovInst (:boolean) $Reg9
// CHKRA-NEXT:  $Reg10 = BranchInst %BB30
// CHKRA-NEXT:%BB37:
// CHKRA-NEXT:  $Reg1 = CondBranchInst $Reg9, %BB55, %BB54
// CHKRA-NEXT:%BB38:
// CHKRA-NEXT:  $Reg11 = LoadPropertyInst (:any) $Reg1, "z": string
// CHKRA-NEXT:  $Reg10 = AllocStackInst (:any) $?anon_20_iter: any
// CHKRA-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_21_sourceOrNext: any
// CHKRA-NEXT:  $Reg11 = StoreStackInst $Reg11, $Reg1
// CHKRA-NEXT:  $Reg11 = IteratorBeginInst (:any) $Reg1
// CHKRA-NEXT:  $Reg11 = StoreStackInst $Reg11, $Reg10
// CHKRA-NEXT:  $Reg11 = TryStartInst %BB40, %BB42
// CHKRA-NEXT:%BB39:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB40, $Reg1, %BB45
// CHKRA-NEXT:  $Reg11 = PhiInst (:undefined|boolean) $Reg11, %BB40, $Reg11, %BB45
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg11 = MovInst (:undefined|boolean) $Reg11
// CHKRA-NEXT:  $Reg11 = CondBranchInst $Reg11, %BB53, %BB52
// CHKRA-NEXT:%BB40:
// CHKRA-NEXT:  $Reg12 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg12
// CHKRA-NEXT:  $Reg11 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg12 = BranchInst %BB39
// CHKRA-NEXT:%BB41:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = IteratorNextInst (:any) $Reg10, $Reg1
// CHKRA-NEXT:  $Reg11 = LoadStackInst (:any) $Reg10
// CHKRA-NEXT:  $Reg11 = BinaryStrictlyEqualInst (:boolean) $Reg11, $Reg0
// CHKRA-NEXT:  $Reg12 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg13 = CondBranchInst $Reg11, %BB44, %BB43
// CHKRA-NEXT:%BB42:
// CHKRA-NEXT:  $Reg11 = TryEndInst %BB40, %BB41
// CHKRA-NEXT:%BB43:
// CHKRA-NEXT:  $Reg12 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = BranchInst %BB44
// CHKRA-NEXT:%BB44:
// CHKRA-NEXT:  $Reg12 = PhiInst (:any) $Reg12, %BB41, $Reg12, %BB43
// CHKRA-NEXT:  $Reg12 = MovInst (:any) $Reg12
// CHKRA-NEXT:  $Reg1 = TryStartInst %BB45, %BB47
// CHKRA-NEXT:%BB45:
// CHKRA-NEXT:  $Reg1 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg11 = MovInst (:boolean) $Reg11
// CHKRA-NEXT:  $Reg12 = BranchInst %BB39
// CHKRA-NEXT:%BB46:
// CHKRA-NEXT:  $Reg1 = CondBranchInst $Reg11, %BB51, %BB50
// CHKRA-NEXT:%BB47:
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:  $Reg1 = CmpBrEqualInst $Reg12, $Reg1, %BB48, %BB49
// CHKRA-NEXT:%BB48:
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:string) "Cannot destructure 'undefined' or 'null'.": string
// CHKRA-NEXT:  $Reg1 = CallBuiltinInst (:any) [HermesBuiltin.throwTypeError]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, $Reg1
// CHKRA-NEXT:  $Reg1 = UnreachableInst
// CHKRA-NEXT:%BB49:
// CHKRA-NEXT:  $Reg1 = TryEndInst %BB45, %BB46
// CHKRA-NEXT:%BB50:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg10
// CHKRA-NEXT:  $Reg1 = IteratorCloseInst (:any) $Reg1, false: boolean
// CHKRA-NEXT:  $Reg1 = BranchInst %BB51
// CHKRA-NEXT:%BB51:
// CHKRA-NEXT:  $Reg1 = TryEndInst %BB36, %BB37
// CHKRA-NEXT:%BB52:
// CHKRA-NEXT:  $Reg10 = LoadStackInst (:any) $Reg10
// CHKRA-NEXT:  $Reg10 = IteratorCloseInst (:any) $Reg10, true: boolean
// CHKRA-NEXT:  $Reg10 = BranchInst %BB53
// CHKRA-NEXT:%BB53:
// CHKRA-NEXT:  $Reg1 = ThrowInst $Reg1, %BB36
// CHKRA-NEXT:%BB54:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg8
// CHKRA-NEXT:  $Reg1 = IteratorCloseInst (:any) $Reg1, false: boolean
// CHKRA-NEXT:  $Reg1 = BranchInst %BB55
// CHKRA-NEXT:%BB55:
// CHKRA-NEXT:  $Reg1 = TryEndInst %BB27, %BB28
// CHKRA-NEXT:%BB56:
// CHKRA-NEXT:  $Reg8 = LoadStackInst (:any) $Reg8
// CHKRA-NEXT:  $Reg8 = IteratorCloseInst (:any) $Reg8, true: boolean
// CHKRA-NEXT:  $Reg8 = BranchInst %BB57
// CHKRA-NEXT:%BB57:
// CHKRA-NEXT:  $Reg1 = ThrowInst $Reg1, %BB27
// CHKRA-NEXT:%BB58:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg6
// CHKRA-NEXT:  $Reg1 = IteratorCloseInst (:any) $Reg1, false: boolean
// CHKRA-NEXT:  $Reg1 = BranchInst %BB59
// CHKRA-NEXT:%BB59:
// CHKRA-NEXT:  $Reg1 = TryEndInst %BB18, %BB19
// CHKRA-NEXT:%BB60:
// CHKRA-NEXT:  $Reg6 = LoadStackInst (:any) $Reg6
// CHKRA-NEXT:  $Reg6 = IteratorCloseInst (:any) $Reg6, true: boolean
// CHKRA-NEXT:  $Reg6 = BranchInst %BB61
// CHKRA-NEXT:%BB61:
// CHKRA-NEXT:  $Reg1 = ThrowInst $Reg1, %BB18
// CHKRA-NEXT:%BB62:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg4
// CHKRA-NEXT:  $Reg1 = IteratorCloseInst (:any) $Reg1, false: boolean
// CHKRA-NEXT:  $Reg1 = BranchInst %BB63
// CHKRA-NEXT:%BB63:
// CHKRA-NEXT:  $Reg1 = TryEndInst %BB9, %BB10
// CHKRA-NEXT:%BB64:
// CHKRA-NEXT:  $Reg4 = LoadStackInst (:any) $Reg4
// CHKRA-NEXT:  $Reg4 = IteratorCloseInst (:any) $Reg4, true: boolean
// CHKRA-NEXT:  $Reg4 = BranchInst %BB65
// CHKRA-NEXT:%BB65:
// CHKRA-NEXT:  $Reg1 = ThrowInst $Reg1, %BB9
// CHKRA-NEXT:%BB66:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg2
// CHKRA-NEXT:  $Reg1 = IteratorCloseInst (:any) $Reg1, false: boolean
// CHKRA-NEXT:  $Reg1 = BranchInst %BB2
// CHKRA-NEXT:%BB67:
// CHKRA-NEXT:  $Reg2 = LoadStackInst (:any) $Reg2
// CHKRA-NEXT:  $Reg2 = IteratorCloseInst (:any) $Reg2, true: boolean
// CHKRA-NEXT:  $Reg2 = BranchInst %BB68
// CHKRA-NEXT:%BB68:
// CHKRA-NEXT:  $Reg1 = ThrowInst $Reg1
// CHKRA-NEXT:function_end

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

// CHKRA:function global(): undefined
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "b": string
// CHKRA-NEXT:  $Reg0 = HBCCreateEnvironmentInst (:any)
// CHKRA-NEXT:  $Reg1 = HBCCreateFunctionInst (:object) %b(): undefined, $Reg0
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "b": string
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function b(d: any): undefined
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg3 = AllocStackInst (:undefined|object) $arguments: any
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg1 = StoreStackInst $Reg0, $Reg3
// CHKRA-NEXT:  $Reg1 = LoadParamInst (:any) %d: any
// CHKRA-NEXT:  $Reg1 = CmpBrStrictlyNotEqualInst $Reg1, $Reg0, %BB1, %BB2
// CHKRA-NEXT:%BB2:
// CHKRA-NEXT:  $Reg2 = AllocStackInst (:any) $?anon_0_iter: any
// CHKRA-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHKRA-NEXT:  $Reg4 = HBCReifyArgumentsLooseInst $Reg3
// CHKRA-NEXT:  $Reg3 = LoadStackInst (:undefined|object) $Reg3
// CHKRA-NEXT:  $Reg3 = StoreStackInst $Reg3, $Reg1
// CHKRA-NEXT:  $Reg3 = IteratorBeginInst (:any) $Reg1
// CHKRA-NEXT:  $Reg3 = StoreStackInst $Reg3, $Reg2
// CHKRA-NEXT:  $Reg3 = TryStartInst %BB3, %BB4
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:%BB5:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB3, $Reg1, %BB6
// CHKRA-NEXT:  $Reg3 = PhiInst (:undefined|boolean) $Reg3, %BB3, $Reg3, %BB6
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg3 = MovInst (:undefined|boolean) $Reg3
// CHKRA-NEXT:  $Reg3 = CondBranchInst $Reg3, %BB7, %BB8
// CHKRA-NEXT:%BB3:
// CHKRA-NEXT:  $Reg4 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg4
// CHKRA-NEXT:  $Reg3 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg4 = BranchInst %BB5
// CHKRA-NEXT:%BB4:
// CHKRA-NEXT:  $Reg3 = BranchInst %BB9
// CHKRA-NEXT:%BB9:
// CHKRA-NEXT:  $Reg3 = TryEndInst
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHKRA-NEXT:  $Reg4 = IteratorNextInst (:any) $Reg2, $Reg1
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg2
// CHKRA-NEXT:  $Reg3 = BinaryStrictlyEqualInst (:boolean) $Reg1, $Reg0
// CHKRA-NEXT:  $Reg1 = MovInst (:undefined) $Reg0
// CHKRA-NEXT:  $Reg5 = CondBranchInst $Reg3, %BB10, %BB11
// CHKRA-NEXT:%BB11:
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg4
// CHKRA-NEXT:  $Reg4 = BranchInst %BB10
// CHKRA-NEXT:%BB10:
// CHKRA-NEXT:  $Reg1 = PhiInst (:any) $Reg1, %BB9, $Reg1, %BB11
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg4 = TryStartInst %BB6, %BB12
// CHKRA-NEXT:%BB6:
// CHKRA-NEXT:  $Reg1 = CatchInst (:any)
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg1
// CHKRA-NEXT:  $Reg3 = MovInst (:boolean) $Reg3
// CHKRA-NEXT:  $Reg4 = BranchInst %BB5
// CHKRA-NEXT:%BB12:
// CHKRA-NEXT:  $Reg4 = AllocStackInst (:any) $?anon_5_iter: any
// CHKRA-NEXT:  $Reg7 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHKRA-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg7
// CHKRA-NEXT:  $Reg1 = IteratorBeginInst (:any) $Reg7
// CHKRA-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg4
// CHKRA-NEXT:  $Reg5 = AllocStackInst (:undefined|boolean) $?anon_7_iterDone: any
// CHKRA-NEXT:  $Reg1 = StoreStackInst $Reg0, $Reg5
// CHKRA-NEXT:  $Reg6 = AllocStackInst (:any) $?anon_8_iterValue: any
// CHKRA-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_9_exc: any
// CHKRA-NEXT:  $Reg8 = TryStartInst %BB13, %BB14
// CHKRA-NEXT:%BB15:
// CHKRA-NEXT:  $Reg5 = LoadStackInst (:undefined|boolean) $Reg5
// CHKRA-NEXT:  $Reg5 = CondBranchInst $Reg5, %BB16, %BB17
// CHKRA-NEXT:%BB13:
// CHKRA-NEXT:  $Reg6 = CatchInst (:any)
// CHKRA-NEXT:  $Reg6 = StoreStackInst $Reg6, $Reg1
// CHKRA-NEXT:  $Reg6 = BranchInst %BB15
// CHKRA-NEXT:%BB14:
// CHKRA-NEXT:  $Reg8 = BranchInst %BB18
// CHKRA-NEXT:%BB18:
// CHKRA-NEXT:  $Reg8 = TryEndInst
// CHKRA-NEXT:  $Reg8 = StoreStackInst $Reg0, $Reg6
// CHKRA-NEXT:  $Reg7 = LoadStackInst (:any) $Reg7
// CHKRA-NEXT:  $Reg7 = IteratorNextInst (:any) $Reg4, $Reg7
// CHKRA-NEXT:  $Reg8 = LoadStackInst (:any) $Reg4
// CHKRA-NEXT:  $Reg8 = BinaryStrictlyEqualInst (:boolean) $Reg8, $Reg0
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg8, $Reg5
// CHKRA-NEXT:  $Reg8 = CondBranchInst $Reg8, %BB19, %BB20
// CHKRA-NEXT:%BB20:
// CHKRA-NEXT:  $Reg7 = StoreStackInst $Reg7, $Reg6
// CHKRA-NEXT:  $Reg7 = BranchInst %BB19
// CHKRA-NEXT:%BB19:
// CHKRA-NEXT:  $Reg7 = TryStartInst %BB21, %BB22
// CHKRA-NEXT:%BB21:
// CHKRA-NEXT:  $Reg6 = CatchInst (:any)
// CHKRA-NEXT:  $Reg6 = StoreStackInst $Reg6, $Reg1
// CHKRA-NEXT:  $Reg6 = BranchInst %BB15
// CHKRA-NEXT:%BB22:
// CHKRA-NEXT:  $Reg6 = LoadStackInst (:any) $Reg6
// CHKRA-NEXT:  $Reg7 = AllocStackInst (:any) $?anon_10_iter: any
// CHKRA-NEXT:  $Reg10 = AllocStackInst (:any) $?anon_11_sourceOrNext: any
// CHKRA-NEXT:  $Reg6 = StoreStackInst $Reg6, $Reg10
// CHKRA-NEXT:  $Reg6 = IteratorBeginInst (:any) $Reg10
// CHKRA-NEXT:  $Reg6 = StoreStackInst $Reg6, $Reg7
// CHKRA-NEXT:  $Reg8 = AllocStackInst (:undefined|boolean) $?anon_12_iterDone: any
// CHKRA-NEXT:  $Reg6 = StoreStackInst $Reg0, $Reg8
// CHKRA-NEXT:  $Reg9 = AllocStackInst (:any) $?anon_13_iterValue: any
// CHKRA-NEXT:  $Reg6 = AllocStackInst (:any) $?anon_14_exc: any
// CHKRA-NEXT:  $Reg11 = TryStartInst %BB23, %BB24
// CHKRA-NEXT:%BB25:
// CHKRA-NEXT:  $Reg8 = LoadStackInst (:undefined|boolean) $Reg8
// CHKRA-NEXT:  $Reg8 = CondBranchInst $Reg8, %BB26, %BB27
// CHKRA-NEXT:%BB23:
// CHKRA-NEXT:  $Reg9 = CatchInst (:any)
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg9, $Reg6
// CHKRA-NEXT:  $Reg9 = BranchInst %BB25
// CHKRA-NEXT:%BB24:
// CHKRA-NEXT:  $Reg11 = BranchInst %BB28
// CHKRA-NEXT:%BB28:
// CHKRA-NEXT:  $Reg11 = TryEndInst
// CHKRA-NEXT:  $Reg11 = StoreStackInst $Reg0, $Reg9
// CHKRA-NEXT:  $Reg10 = LoadStackInst (:any) $Reg10
// CHKRA-NEXT:  $Reg10 = IteratorNextInst (:any) $Reg7, $Reg10
// CHKRA-NEXT:  $Reg11 = LoadStackInst (:any) $Reg7
// CHKRA-NEXT:  $Reg11 = BinaryStrictlyEqualInst (:boolean) $Reg11, $Reg0
// CHKRA-NEXT:  $Reg12 = StoreStackInst $Reg11, $Reg8
// CHKRA-NEXT:  $Reg11 = CondBranchInst $Reg11, %BB29, %BB30
// CHKRA-NEXT:%BB30:
// CHKRA-NEXT:  $Reg10 = StoreStackInst $Reg10, $Reg9
// CHKRA-NEXT:  $Reg10 = BranchInst %BB29
// CHKRA-NEXT:%BB29:
// CHKRA-NEXT:  $Reg10 = TryStartInst %BB31, %BB32
// CHKRA-NEXT:%BB31:
// CHKRA-NEXT:  $Reg9 = CatchInst (:any)
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg9, $Reg6
// CHKRA-NEXT:  $Reg9 = BranchInst %BB25
// CHKRA-NEXT:%BB32:
// CHKRA-NEXT:  $Reg9 = LoadStackInst (:any) $Reg9
// CHKRA-NEXT:  $Reg10 = AllocStackInst (:any) $?anon_15_iter: any
// CHKRA-NEXT:  $Reg13 = AllocStackInst (:any) $?anon_16_sourceOrNext: any
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg9, $Reg13
// CHKRA-NEXT:  $Reg9 = IteratorBeginInst (:any) $Reg13
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg9, $Reg10
// CHKRA-NEXT:  $Reg11 = AllocStackInst (:undefined|boolean) $?anon_17_iterDone: any
// CHKRA-NEXT:  $Reg9 = StoreStackInst $Reg0, $Reg11
// CHKRA-NEXT:  $Reg12 = AllocStackInst (:any) $?anon_18_iterValue: any
// CHKRA-NEXT:  $Reg9 = AllocStackInst (:any) $?anon_19_exc: any
// CHKRA-NEXT:  $Reg14 = TryStartInst %BB33, %BB34
// CHKRA-NEXT:%BB35:
// CHKRA-NEXT:  $Reg11 = LoadStackInst (:undefined|boolean) $Reg11
// CHKRA-NEXT:  $Reg11 = CondBranchInst $Reg11, %BB36, %BB37
// CHKRA-NEXT:%BB33:
// CHKRA-NEXT:  $Reg12 = CatchInst (:any)
// CHKRA-NEXT:  $Reg12 = StoreStackInst $Reg12, $Reg9
// CHKRA-NEXT:  $Reg12 = BranchInst %BB35
// CHKRA-NEXT:%BB34:
// CHKRA-NEXT:  $Reg14 = BranchInst %BB38
// CHKRA-NEXT:%BB38:
// CHKRA-NEXT:  $Reg14 = TryEndInst
// CHKRA-NEXT:  $Reg14 = StoreStackInst $Reg0, $Reg12
// CHKRA-NEXT:  $Reg13 = LoadStackInst (:any) $Reg13
// CHKRA-NEXT:  $Reg13 = IteratorNextInst (:any) $Reg10, $Reg13
// CHKRA-NEXT:  $Reg14 = LoadStackInst (:any) $Reg10
// CHKRA-NEXT:  $Reg14 = BinaryStrictlyEqualInst (:boolean) $Reg14, $Reg0
// CHKRA-NEXT:  $Reg15 = StoreStackInst $Reg14, $Reg11
// CHKRA-NEXT:  $Reg14 = CondBranchInst $Reg14, %BB39, %BB40
// CHKRA-NEXT:%BB40:
// CHKRA-NEXT:  $Reg13 = StoreStackInst $Reg13, $Reg12
// CHKRA-NEXT:  $Reg13 = BranchInst %BB39
// CHKRA-NEXT:%BB39:
// CHKRA-NEXT:  $Reg13 = TryStartInst %BB41, %BB42
// CHKRA-NEXT:%BB41:
// CHKRA-NEXT:  $Reg12 = CatchInst (:any)
// CHKRA-NEXT:  $Reg12 = StoreStackInst $Reg12, $Reg9
// CHKRA-NEXT:  $Reg12 = BranchInst %BB35
// CHKRA-NEXT:%BB42:
// CHKRA-NEXT:  $Reg12 = LoadStackInst (:any) $Reg12
// CHKRA-NEXT:  $Reg12 = LoadPropertyInst (:any) $Reg12, "z": string
// CHKRA-NEXT:  $Reg13 = AllocStackInst (:any) $?anon_20_iter: any
// CHKRA-NEXT:  $Reg16 = AllocStackInst (:any) $?anon_21_sourceOrNext: any
// CHKRA-NEXT:  $Reg12 = StoreStackInst $Reg12, $Reg16
// CHKRA-NEXT:  $Reg12 = IteratorBeginInst (:any) $Reg16
// CHKRA-NEXT:  $Reg12 = StoreStackInst $Reg12, $Reg13
// CHKRA-NEXT:  $Reg14 = AllocStackInst (:undefined|boolean) $?anon_22_iterDone: any
// CHKRA-NEXT:  $Reg12 = StoreStackInst $Reg0, $Reg14
// CHKRA-NEXT:  $Reg15 = AllocStackInst (:any) $?anon_23_iterValue: any
// CHKRA-NEXT:  $Reg12 = AllocStackInst (:any) $?anon_24_exc: any
// CHKRA-NEXT:  $Reg17 = TryStartInst %BB43, %BB44
// CHKRA-NEXT:%BB45:
// CHKRA-NEXT:  $Reg14 = LoadStackInst (:undefined|boolean) $Reg14
// CHKRA-NEXT:  $Reg14 = CondBranchInst $Reg14, %BB46, %BB47
// CHKRA-NEXT:%BB43:
// CHKRA-NEXT:  $Reg15 = CatchInst (:any)
// CHKRA-NEXT:  $Reg15 = StoreStackInst $Reg15, $Reg12
// CHKRA-NEXT:  $Reg15 = BranchInst %BB45
// CHKRA-NEXT:%BB44:
// CHKRA-NEXT:  $Reg17 = BranchInst %BB48
// CHKRA-NEXT:%BB48:
// CHKRA-NEXT:  $Reg17 = TryEndInst
// CHKRA-NEXT:  $Reg17 = StoreStackInst $Reg0, $Reg15
// CHKRA-NEXT:  $Reg16 = LoadStackInst (:any) $Reg16
// CHKRA-NEXT:  $Reg16 = IteratorNextInst (:any) $Reg13, $Reg16
// CHKRA-NEXT:  $Reg17 = LoadStackInst (:any) $Reg13
// CHKRA-NEXT:  $Reg17 = BinaryStrictlyEqualInst (:boolean) $Reg17, $Reg0
// CHKRA-NEXT:  $Reg18 = StoreStackInst $Reg17, $Reg14
// CHKRA-NEXT:  $Reg17 = CondBranchInst $Reg17, %BB49, %BB50
// CHKRA-NEXT:%BB50:
// CHKRA-NEXT:  $Reg16 = StoreStackInst $Reg16, $Reg15
// CHKRA-NEXT:  $Reg16 = BranchInst %BB49
// CHKRA-NEXT:%BB49:
// CHKRA-NEXT:  $Reg16 = TryStartInst %BB51, %BB52
// CHKRA-NEXT:%BB51:
// CHKRA-NEXT:  $Reg15 = CatchInst (:any)
// CHKRA-NEXT:  $Reg15 = StoreStackInst $Reg15, $Reg12
// CHKRA-NEXT:  $Reg15 = BranchInst %BB45
// CHKRA-NEXT:%BB52:
// CHKRA-NEXT:  $Reg16 = LoadStackInst (:any) $Reg15
// CHKRA-NEXT:  $Reg15 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:  $Reg15 = CmpBrEqualInst $Reg16, $Reg15, %BB53, %BB54
// CHKRA-NEXT:%BB53:
// CHKRA-NEXT:  $Reg15 = HBCLoadConstInst (:string) "Cannot destructure 'undefined' or 'null'.": string
// CHKRA-NEXT:  $Reg15 = CallBuiltinInst (:any) [HermesBuiltin.throwTypeError]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, $Reg15
// CHKRA-NEXT:  $Reg15 = LIRDeadTerminatorInst
// CHKRA-NEXT:%BB54:
// CHKRA-NEXT:  $Reg15 = BranchInst %BB55
// CHKRA-NEXT:%BB55:
// CHKRA-NEXT:  $Reg15 = TryEndInst
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:undefined|boolean) $Reg14
// CHKRA-NEXT:  $Reg15 = CondBranchInst $Reg15, %BB56, %BB57
// CHKRA-NEXT:%BB57:
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:any) $Reg13
// CHKRA-NEXT:  $Reg15 = IteratorCloseInst (:any) $Reg15, false: boolean
// CHKRA-NEXT:  $Reg15 = BranchInst %BB56
// CHKRA-NEXT:%BB56:
// CHKRA-NEXT:  $Reg15 = BranchInst %BB58
// CHKRA-NEXT:%BB47:
// CHKRA-NEXT:  $Reg13 = LoadStackInst (:any) $Reg13
// CHKRA-NEXT:  $Reg13 = IteratorCloseInst (:any) $Reg13, true: boolean
// CHKRA-NEXT:  $Reg13 = BranchInst %BB46
// CHKRA-NEXT:%BB46:
// CHKRA-NEXT:  $Reg12 = LoadStackInst (:any) $Reg12
// CHKRA-NEXT:  $Reg12 = ThrowInst $Reg12
// CHKRA-NEXT:%BB58:
// CHKRA-NEXT:  $Reg15 = TryEndInst
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:undefined|boolean) $Reg11
// CHKRA-NEXT:  $Reg15 = CondBranchInst $Reg15, %BB59, %BB60
// CHKRA-NEXT:%BB60:
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:any) $Reg10
// CHKRA-NEXT:  $Reg15 = IteratorCloseInst (:any) $Reg15, false: boolean
// CHKRA-NEXT:  $Reg15 = BranchInst %BB59
// CHKRA-NEXT:%BB59:
// CHKRA-NEXT:  $Reg15 = BranchInst %BB61
// CHKRA-NEXT:%BB37:
// CHKRA-NEXT:  $Reg10 = LoadStackInst (:any) $Reg10
// CHKRA-NEXT:  $Reg10 = IteratorCloseInst (:any) $Reg10, true: boolean
// CHKRA-NEXT:  $Reg10 = BranchInst %BB36
// CHKRA-NEXT:%BB36:
// CHKRA-NEXT:  $Reg9 = LoadStackInst (:any) $Reg9
// CHKRA-NEXT:  $Reg9 = ThrowInst $Reg9
// CHKRA-NEXT:%BB61:
// CHKRA-NEXT:  $Reg15 = TryEndInst
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:undefined|boolean) $Reg8
// CHKRA-NEXT:  $Reg15 = CondBranchInst $Reg15, %BB62, %BB63
// CHKRA-NEXT:%BB63:
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:any) $Reg7
// CHKRA-NEXT:  $Reg15 = IteratorCloseInst (:any) $Reg15, false: boolean
// CHKRA-NEXT:  $Reg15 = BranchInst %BB62
// CHKRA-NEXT:%BB62:
// CHKRA-NEXT:  $Reg15 = BranchInst %BB64
// CHKRA-NEXT:%BB27:
// CHKRA-NEXT:  $Reg7 = LoadStackInst (:any) $Reg7
// CHKRA-NEXT:  $Reg7 = IteratorCloseInst (:any) $Reg7, true: boolean
// CHKRA-NEXT:  $Reg7 = BranchInst %BB26
// CHKRA-NEXT:%BB26:
// CHKRA-NEXT:  $Reg6 = LoadStackInst (:any) $Reg6
// CHKRA-NEXT:  $Reg6 = ThrowInst $Reg6
// CHKRA-NEXT:%BB64:
// CHKRA-NEXT:  $Reg15 = TryEndInst
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:undefined|boolean) $Reg5
// CHKRA-NEXT:  $Reg15 = CondBranchInst $Reg15, %BB65, %BB66
// CHKRA-NEXT:%BB66:
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:any) $Reg4
// CHKRA-NEXT:  $Reg15 = IteratorCloseInst (:any) $Reg15, false: boolean
// CHKRA-NEXT:  $Reg15 = BranchInst %BB65
// CHKRA-NEXT:%BB65:
// CHKRA-NEXT:  $Reg15 = BranchInst %BB67
// CHKRA-NEXT:%BB17:
// CHKRA-NEXT:  $Reg4 = LoadStackInst (:any) $Reg4
// CHKRA-NEXT:  $Reg4 = IteratorCloseInst (:any) $Reg4, true: boolean
// CHKRA-NEXT:  $Reg4 = BranchInst %BB16
// CHKRA-NEXT:%BB16:
// CHKRA-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHKRA-NEXT:  $Reg1 = ThrowInst $Reg1
// CHKRA-NEXT:%BB67:
// CHKRA-NEXT:  $Reg15 = TryEndInst
// CHKRA-NEXT:  $Reg15 = CondBranchInst $Reg3, %BB1, %BB68
// CHKRA-NEXT:%BB68:
// CHKRA-NEXT:  $Reg15 = LoadStackInst (:any) $Reg2
// CHKRA-NEXT:  $Reg15 = IteratorCloseInst (:any) $Reg15, false: boolean
// CHKRA-NEXT:  $Reg15 = BranchInst %BB1
// CHKRA-NEXT:%BB8:
// CHKRA-NEXT:  $Reg2 = LoadStackInst (:any) $Reg2
// CHKRA-NEXT:  $Reg2 = IteratorCloseInst (:any) $Reg2, true: boolean
// CHKRA-NEXT:  $Reg2 = BranchInst %BB7
// CHKRA-NEXT:%BB7:
// CHKRA-NEXT:  $Reg1 = ThrowInst $Reg1
// CHKRA-NEXT:function_end

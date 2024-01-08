/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

var a = [];
var x = {};
var y = {}

for (var i=0 ; i < 3; ++i) {
  y = {}

  for (a[2] in x) {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined|object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "a": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "x": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "y": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "i": string
// CHECK-NEXT:  $Reg0 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  $Reg4 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg4, "a": string
// CHECK-NEXT:  $Reg0 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg4, "x": string
// CHECK-NEXT:  $Reg0 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg4, "y": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg4, "i": string
// CHECK-NEXT:  $Reg0 = LoadPropertyInst (:any) $Reg4, "i": string
// CHECK-NEXT:  $Reg3 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg1 = BinaryLessThanInst (:boolean) $Reg0, $Reg3
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg0 = MovInst (:undefined) $Reg0
// CHECK-NEXT:  $Reg1 = CondBranchInst $Reg1, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg5 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  $Reg1 = StorePropertyLooseInst $Reg5, $Reg4, "y": string
// CHECK-NEXT:  $Reg9 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  $Reg8 = AllocStackInst (:any) $?anon_2_base: any
// CHECK-NEXT:  $Reg7 = AllocStackInst (:number) $?anon_3_idx: any
// CHECK-NEXT:  $Reg6 = AllocStackInst (:number) $?anon_4_size: any
// CHECK-NEXT:  $Reg1 = LoadPropertyInst (:any) $Reg4, "x": string
// CHECK-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg8
// CHECK-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_5_prop: any
// CHECK-NEXT:  $Reg10 = GetPNamesInst $Reg9, $Reg8, $Reg7, $Reg6, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = PhiInst (:undefined|object) $Reg0, %BB0, $Reg0, %BB3
// CHECK-NEXT:  $Reg0 = MovInst (:undefined|object) $Reg0
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg1 = LoadPropertyInst (:any) $Reg4, "i": string
// CHECK-NEXT:  $Reg1 = UnaryIncInst (:number|bigint) $Reg1
// CHECK-NEXT:  $Reg1 = StorePropertyLooseInst $Reg1, $Reg4, "i": string
// CHECK-NEXT:  $Reg1 = LoadPropertyInst (:any) $Reg4, "i": string
// CHECK-NEXT:  $Reg0 = MovInst (:object) $Reg5
// CHECK-NEXT:  $Reg1 = CmpBrLessThanInst $Reg1, $Reg3, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg10 = GetNextPNameInst $Reg1, $Reg8, $Reg7, $Reg6, $Reg9, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg11 = LoadStackInst (:any) $Reg1
// CHECK-NEXT:  $Reg10 = LoadPropertyInst (:any) $Reg4, "a": string
// CHECK-NEXT:  $Reg10 = StorePropertyLooseInst $Reg11, $Reg10, $Reg2
// CHECK-NEXT:  $Reg1 = BranchInst %BB4
// CHECK-NEXT:function_end

/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Ensure there's only one copy of the C constructor.
try {} finally { class C {} }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = [C: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:       StoreStackInst "use strict": string, %0: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:       StoreFrameInst %6: object, [C]: any
// CHECK-NEXT:  %8 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StorePropertyStrictInst %8: object, %6: object, "prototype": string
// CHECK-NEXT:        ThrowInst %4: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %0: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:        StoreFrameInst %16: object, [C]: any
// CHECK-NEXT:  %18 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StorePropertyStrictInst %18: object, %16: object, "prototype": string
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

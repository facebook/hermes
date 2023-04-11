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
// CHECK-NEXT:  %1 = StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = StoreStackInst "use strict": string, %0: any
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:  %7 = StoreFrameInst %6: object, [C]: any
// CHECK-NEXT:  %8 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8: object, %6: object, "prototype": string
// CHECK-NEXT:  %10 = ThrowInst %4: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = TryEndInst
// CHECK-NEXT:  %15 = StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:  %17 = StoreFrameInst %16: object, [C]: any
// CHECK-NEXT:  %18 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %19 = StorePropertyStrictInst %18: object, %16: object, "prototype": string
// CHECK-NEXT:  %20 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

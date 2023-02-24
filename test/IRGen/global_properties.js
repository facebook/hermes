/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

Object
Function
Array
String
Boolean
Number
Math
Date
RegExp
Error
JSON

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %1 = StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Object": string
// CHECK-NEXT:  %3 = StoreStackInst %2: any, %0: any
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Function": string
// CHECK-NEXT:  %5 = StoreStackInst %4: any, %0: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Array": string
// CHECK-NEXT:  %7 = StoreStackInst %6: any, %0: any
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "String": string
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %0: any
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Boolean": string
// CHECK-NEXT:  %11 = StoreStackInst %10: any, %0: any
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Number": string
// CHECK-NEXT:  %13 = StoreStackInst %12: any, %0: any
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// CHECK-NEXT:  %15 = StoreStackInst %14: any, %0: any
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Date": string
// CHECK-NEXT:  %17 = StoreStackInst %16: any, %0: any
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "RegExp": string
// CHECK-NEXT:  %19 = StoreStackInst %18: any, %0: any
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Error": string
// CHECK-NEXT:  %21 = StoreStackInst %20: any, %0: any
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst (:any) globalObject: object, "JSON": string
// CHECK-NEXT:  %23 = StoreStackInst %22: any, %0: any
// CHECK-NEXT:  %24 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %25 = ReturnInst %24: any
// CHECK-NEXT:function_end

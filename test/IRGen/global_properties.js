/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "Object" : string
// CHECK-NEXT:  %3 = StoreStackInst %2, %0
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "Function" : string
// CHECK-NEXT:  %5 = StoreStackInst %4, %0
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "Array" : string
// CHECK-NEXT:  %7 = StoreStackInst %6, %0
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "String" : string
// CHECK-NEXT:  %9 = StoreStackInst %8, %0
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst globalObject : object, "Boolean" : string
// CHECK-NEXT:  %11 = StoreStackInst %10, %0
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst globalObject : object, "Number" : string
// CHECK-NEXT:  %13 = StoreStackInst %12, %0
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "Math" : string
// CHECK-NEXT:  %15 = StoreStackInst %14, %0
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst globalObject : object, "Date" : string
// CHECK-NEXT:  %17 = StoreStackInst %16, %0
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst globalObject : object, "RegExp" : string
// CHECK-NEXT:  %19 = StoreStackInst %18, %0
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst globalObject : object, "Error" : string
// CHECK-NEXT:  %21 = StoreStackInst %20, %0
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst globalObject : object, "JSON" : string
// CHECK-NEXT:  %23 = StoreStackInst %22, %0
// CHECK-NEXT:  %24 = LoadStackInst %0
// CHECK-NEXT:  %25 = ReturnInst %24
// CHECK-NEXT:function_end

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "Object" : string
// CHECK-NEXT:  %4 = StoreStackInst %3, %1
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "Function" : string
// CHECK-NEXT:  %6 = StoreStackInst %5, %1
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "Array" : string
// CHECK-NEXT:  %8 = StoreStackInst %7, %1
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst globalObject : object, "String" : string
// CHECK-NEXT:  %10 = StoreStackInst %9, %1
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst globalObject : object, "Boolean" : string
// CHECK-NEXT:  %12 = StoreStackInst %11, %1
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst globalObject : object, "Number" : string
// CHECK-NEXT:  %14 = StoreStackInst %13, %1
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst globalObject : object, "Math" : string
// CHECK-NEXT:  %16 = StoreStackInst %15, %1
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst globalObject : object, "Date" : string
// CHECK-NEXT:  %18 = StoreStackInst %17, %1
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst globalObject : object, "RegExp" : string
// CHECK-NEXT:  %20 = StoreStackInst %19, %1
// CHECK-NEXT:  %21 = TryLoadGlobalPropertyInst globalObject : object, "Error" : string
// CHECK-NEXT:  %22 = StoreStackInst %21, %1
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst globalObject : object, "JSON" : string
// CHECK-NEXT:  %24 = StoreStackInst %23, %1
// CHECK-NEXT:  %25 = LoadStackInst %1
// CHECK-NEXT:  %26 = ReturnInst %25
// CHECK-NEXT:function_end

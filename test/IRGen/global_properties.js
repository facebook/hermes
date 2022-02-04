/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK: function global()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = AllocStackInst $?anon_0_ret
//CHECK:     %1 = StoreStackInst undefined : undefined, %0
//CHECK:     %2 = TryLoadGlobalPropertyInst globalObject : object, "Object" : string
//CHECK:     %3 = StoreStackInst %2, %0
//CHECK:     %4 = TryLoadGlobalPropertyInst globalObject : object, "Function" : string
//CHECK:     %5 = StoreStackInst %4, %0
//CHECK:     %6 = TryLoadGlobalPropertyInst globalObject : object, "Array" : string
//CHECK:     %7 = StoreStackInst %6, %0
//CHECK:     %8 = TryLoadGlobalPropertyInst globalObject : object, "String" : string
//CHECK:     %9 = StoreStackInst %8, %0
//CHECK:     %10 = TryLoadGlobalPropertyInst globalObject : object, "Boolean" : string
//CHECK:     %11 = StoreStackInst %10, %0
//CHECK:     %12 = TryLoadGlobalPropertyInst globalObject : object, "Number" : string
//CHECK:     %13 = StoreStackInst %12, %0
//CHECK:     %14 = TryLoadGlobalPropertyInst globalObject : object, "Math" : string
//CHECK:     %15 = StoreStackInst %14, %0
//CHECK:     %16 = TryLoadGlobalPropertyInst globalObject : object, "Date" : string
//CHECK:     %17 = StoreStackInst %16, %0
//CHECK:     %18 = TryLoadGlobalPropertyInst globalObject : object, "RegExp" : string
//CHECK:     %19 = StoreStackInst %18, %0
//CHECK:     %20 = TryLoadGlobalPropertyInst globalObject : object, "Error" : string
//CHECK:     %21 = StoreStackInst %20, %0
//CHECK:     %22 = TryLoadGlobalPropertyInst globalObject : object, "JSON" : string
//CHECK:     %23 = StoreStackInst %22, %0
//CHECK:     %24 = LoadStackInst %0
//CHECK:     %25 = ReturnInst %24
//CHECK: function_end

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

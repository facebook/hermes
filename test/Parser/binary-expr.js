/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -hermes-parser -dump-ir %s | %FileCheck %s --match-full-lines

var r, a, b, c, d, e;
//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [r, a, b, c, d, e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0

r = a + b * c - d;
//CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "b" : string
//CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "c" : string
//CHECK-NEXT:  %5 = BinaryOperatorInst '*', %3, %4
//CHECK-NEXT:  %6 = BinaryOperatorInst '+', %2, %5
//CHECK-NEXT:  %7 = LoadPropertyInst globalObject : object, "d" : string
//CHECK-NEXT:  %8 = BinaryOperatorInst '-', %6, %7
//CHECK-NEXT:  %9 = StorePropertyInst %8, globalObject : object, "r" : string
//CHECK-NEXT:  %10 = StoreStackInst %8, %0

r = a + b * c / d + e;
//CHECK-NEXT:  %11 = LoadPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:  %12 = LoadPropertyInst globalObject : object, "b" : string
//CHECK-NEXT:  %13 = LoadPropertyInst globalObject : object, "c" : string
//CHECK-NEXT:  %14 = BinaryOperatorInst '*', %12, %13
//CHECK-NEXT:  %15 = LoadPropertyInst globalObject : object, "d" : string
//CHECK-NEXT:  %16 = BinaryOperatorInst '/', %14, %15
//CHECK-NEXT:  %17 = BinaryOperatorInst '+', %11, %16
//CHECK-NEXT:  %18 = LoadPropertyInst globalObject : object, "e" : string
//CHECK-NEXT:  %19 = BinaryOperatorInst '+', %17, %18
//CHECK-NEXT:  %20 = StorePropertyInst %19, globalObject : object, "r" : string
//CHECK-NEXT:  %21 = StoreStackInst %19, %0

r = a * b + c * d;
//CHECK-NEXT:  %22 = LoadPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:  %23 = LoadPropertyInst globalObject : object, "b" : string
//CHECK-NEXT:  %24 = BinaryOperatorInst '*', %22, %23
//CHECK-NEXT:  %25 = LoadPropertyInst globalObject : object, "c" : string
//CHECK-NEXT:  %26 = LoadPropertyInst globalObject : object, "d" : string
//CHECK-NEXT:  %27 = BinaryOperatorInst '*', %25, %26
//CHECK-NEXT:  %28 = BinaryOperatorInst '+', %24, %27
//CHECK-NEXT:  %29 = StorePropertyInst %28, globalObject : object, "r" : string
//CHECK-NEXT:  %30 = StoreStackInst %28, %0

r = a * b + c - d;
//CHECK-NEXT:  %31 = LoadPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:  %32 = LoadPropertyInst globalObject : object, "b" : string
//CHECK-NEXT:  %33 = BinaryOperatorInst '*', %31, %32
//CHECK-NEXT:  %34 = LoadPropertyInst globalObject : object, "c" : string
//CHECK-NEXT:  %35 = BinaryOperatorInst '+', %33, %34
//CHECK-NEXT:  %36 = LoadPropertyInst globalObject : object, "d" : string
//CHECK-NEXT:  %37 = BinaryOperatorInst '-', %35, %36
//CHECK-NEXT:  %38 = StorePropertyInst %37, globalObject : object, "r" : string
//CHECK-NEXT:  %39 = StoreStackInst %37, %0

// 'in' precedence is handled separately from TokenKinds.def and easy to miss.
r = a in b !== c in d;
//CHECK-NEXT:  %40 = LoadPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:  %41 = LoadPropertyInst globalObject : object, "b" : string
//CHECK-NEXT:  %42 = BinaryOperatorInst 'in', %40, %41
//CHECK-NEXT:  %43 = LoadPropertyInst globalObject : object, "c" : string
//CHECK-NEXT:  %44 = LoadPropertyInst globalObject : object, "d" : string
//CHECK-NEXT:  %45 = BinaryOperatorInst 'in', %43, %44
//CHECK-NEXT:  %46 = BinaryOperatorInst '!==', %42, %45
//CHECK-NEXT:  %47 = StorePropertyInst %46, globalObject : object, "r" : string
//CHECK-NEXT:  %48 = StoreStackInst %46, %0

//CHECK-NEXT:  %49 = LoadStackInst %0
//CHECK-NEXT:  %50 = ReturnInst %49
//CHECK-NEXT:function_end

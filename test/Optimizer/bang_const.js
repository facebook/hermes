/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s

print(!0);

print(!-0);

print(!0.0);

print(!NaN);

print(!1);

print(!null);

print(!undefined);

print(!"");

print(!"abc");

print(!print);

print(!true);

print(!false);

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined, true : boolean
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined, true : boolean
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "NaN" : string
// CHECK-NEXT:  %9 = UnaryOperatorInst '!', %8
// CHECK-NEXT:  %10 = CallInst %7, undefined : undefined, %9 : boolean
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %12 = CallInst %11, undefined : undefined, false : boolean
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %14 = CallInst %13, undefined : undefined, true : boolean
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %16 = CallInst %15, undefined : undefined, true : boolean
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %18 = CallInst %17, undefined : undefined, true : boolean
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %20 = CallInst %19, undefined : undefined, false : boolean
// CHECK-NEXT:  %21 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %23 = UnaryOperatorInst '!', %22
// CHECK-NEXT:  %24 = CallInst %21, undefined : undefined, %23 : boolean
// CHECK-NEXT:  %25 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %26 = CallInst %25, undefined : undefined, false : boolean
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %28 = CallInst %27, undefined : undefined, true : boolean
// CHECK-NEXT:  %29 = ReturnInst %28
// CHECK-NEXT:function_end

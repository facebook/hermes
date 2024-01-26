/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines --check-prefix=NOOPT
// RUN: %hermesc -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT

print(Object);
print(Function);
print(Array);
print(String);
print(Boolean);
print(Number);
print(Math);
print(Date);
print(RegExp);
print(Error);
print(JSON);
print(Infinity);
print(NaN);
print(undefined);

// Auto-generated content below. Please do not modify manually.

// NOOPT:function global(): any
// NOOPT-NEXT:frame = []
// NOOPT-NEXT:%BB0:
// NOOPT-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// NOOPT-NEXT:       StoreStackInst undefined: undefined, %0: any
// NOOPT-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Object": string
// NOOPT-NEXT:  %4 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: any
// NOOPT-NEXT:       StoreStackInst %4: any, %0: any
// NOOPT-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Function": string
// NOOPT-NEXT:  %8 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %7: any
// NOOPT-NEXT:       StoreStackInst %8: any, %0: any
// NOOPT-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Array": string
// NOOPT-NEXT:  %12 = CallInst (:any) %10: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: any
// NOOPT-NEXT:        StoreStackInst %12: any, %0: any
// NOOPT-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "String": string
// NOOPT-NEXT:  %16 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %15: any
// NOOPT-NEXT:        StoreStackInst %16: any, %0: any
// NOOPT-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %19 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Boolean": string
// NOOPT-NEXT:  %20 = CallInst (:any) %18: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %19: any
// NOOPT-NEXT:        StoreStackInst %20: any, %0: any
// NOOPT-NEXT:  %22 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Number": string
// NOOPT-NEXT:  %24 = CallInst (:any) %22: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %23: any
// NOOPT-NEXT:        StoreStackInst %24: any, %0: any
// NOOPT-NEXT:  %26 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %27 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// NOOPT-NEXT:  %28 = CallInst (:any) %26: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %27: any
// NOOPT-NEXT:        StoreStackInst %28: any, %0: any
// NOOPT-NEXT:  %30 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %31 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Date": string
// NOOPT-NEXT:  %32 = CallInst (:any) %30: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %31: any
// NOOPT-NEXT:        StoreStackInst %32: any, %0: any
// NOOPT-NEXT:  %34 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %35 = TryLoadGlobalPropertyInst (:any) globalObject: object, "RegExp": string
// NOOPT-NEXT:  %36 = CallInst (:any) %34: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %35: any
// NOOPT-NEXT:        StoreStackInst %36: any, %0: any
// NOOPT-NEXT:  %38 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %39 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Error": string
// NOOPT-NEXT:  %40 = CallInst (:any) %38: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %39: any
// NOOPT-NEXT:        StoreStackInst %40: any, %0: any
// NOOPT-NEXT:  %42 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %43 = TryLoadGlobalPropertyInst (:any) globalObject: object, "JSON": string
// NOOPT-NEXT:  %44 = CallInst (:any) %42: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %43: any
// NOOPT-NEXT:        StoreStackInst %44: any, %0: any
// NOOPT-NEXT:  %46 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %47 = CallInst (:any) %46: any, empty: any, empty: any, undefined: undefined, undefined: undefined, Infinity: number
// NOOPT-NEXT:        StoreStackInst %47: any, %0: any
// NOOPT-NEXT:  %49 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %50 = CallInst (:any) %49: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// NOOPT-NEXT:        StoreStackInst %50: any, %0: any
// NOOPT-NEXT:  %52 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// NOOPT-NEXT:  %53 = CallInst (:any) %52: any, empty: any, empty: any, undefined: undefined, undefined: undefined, undefined: undefined
// NOOPT-NEXT:        StoreStackInst %53: any, %0: any
// NOOPT-NEXT:  %55 = LoadStackInst (:any) %0: any
// NOOPT-NEXT:        ReturnInst %55: any
// NOOPT-NEXT:function_end

// OPT:function global(): any
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Object": string
// OPT-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// OPT-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Function": string
// OPT-NEXT:  %5 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: any
// OPT-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Array": string
// OPT-NEXT:  %8 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %7: any
// OPT-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "String": string
// OPT-NEXT:  %11 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: any
// OPT-NEXT:  %12 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %13 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Boolean": string
// OPT-NEXT:  %14 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: any
// OPT-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Number": string
// OPT-NEXT:  %17 = CallInst (:any) %15: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %16: any
// OPT-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %19 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// OPT-NEXT:  %20 = CallInst (:any) %18: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %19: any
// OPT-NEXT:  %21 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %22 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Date": string
// OPT-NEXT:  %23 = CallInst (:any) %21: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %22: any
// OPT-NEXT:  %24 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %25 = TryLoadGlobalPropertyInst (:any) globalObject: object, "RegExp": string
// OPT-NEXT:  %26 = CallInst (:any) %24: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %25: any
// OPT-NEXT:  %27 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %28 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Error": string
// OPT-NEXT:  %29 = CallInst (:any) %27: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %28: any
// OPT-NEXT:  %30 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %31 = TryLoadGlobalPropertyInst (:any) globalObject: object, "JSON": string
// OPT-NEXT:  %32 = CallInst (:any) %30: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %31: any
// OPT-NEXT:  %33 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %34 = CallInst (:any) %33: any, empty: any, empty: any, undefined: undefined, undefined: undefined, Infinity: number
// OPT-NEXT:  %35 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %36 = CallInst (:any) %35: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// OPT-NEXT:  %37 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NEXT:  %38 = CallInst (:any) %37: any, empty: any, empty: any, undefined: undefined, undefined: undefined, undefined: undefined
// OPT-NEXT:        ReturnInst %38: any
// OPT-NEXT:function_end

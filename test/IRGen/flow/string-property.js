/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -Xdump-functions=main -fno-std-globals -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

function main() {
  let s: string = "hello";
  let len: number = s.length;
  let c: string = s[0];
  print(s, len, c);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [exports: any, main: any]

// CHECK:scope %VS1 [s: any, len: any, c: any]

// CHECK:function main(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.s]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.len]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.c]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, "hello": string, [%VS1.s]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS1.s]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:string) %6: any, type(string)
// CHECK-NEXT:  %8 = LoadPropertyInst (:number) %7: string, "length": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: number, [%VS1.len]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS1.s]: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:string) %10: any, type(string)
// CHECK-NEXT:  %12 = LoadPropertyInst (:undefined|string) %11: string, 0: number
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:string) %12: undefined|string, type(string)
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: string, [%VS1.c]: any
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS1.s]: any
// CHECK-NEXT:  %17 = CheckedTypeCastInst (:string) %16: any, type(string)
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [%VS1.len]: any
// CHECK-NEXT:  %19 = CheckedTypeCastInst (:number) %18: any, type(number)
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [%VS1.c]: any
// CHECK-NEXT:  %21 = CheckedTypeCastInst (:string) %20: any, type(string)
// CHECK-NEXT:  %22 = CallInst (:any) %15: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %17: string, %19: number, %21: string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

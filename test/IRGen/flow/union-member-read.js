/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -typed -dump-ir -Xdump-functions=tagOf,valueOf,slotOf %s | %FileCheckOrRegen %s --match-full-lines

// Reading a field common to every arm of an object-literal union. When the
// field is at the same slot in every arm it lowers to a single PrLoadInst at
// that slot; when the slots differ it lowers to a by-name LoadPropertyInst.

// Discriminant read: result is the union of the literal arm types (string).
function tagOf(s: {tag: 'circle', radius: number} | {tag: 'square', side: number}): string {
  return s.tag;
}

// Differing field types at the same slot: the load is typed as the union.
function valueOf(p: {k: number} | {k: string}): number | string {
  return p.k;
}

// Field at a different slot in each arm: lowers to a by-name LoadPropertyInst.
function slotOf(p: {tag: string, x: number} | {x: number, tag: string}): string {
  return p.tag;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [exports: any, tagOf: any, valueOf: any, slotOf: any]

// CHECK:scope %VS1 [s: any]

// CHECK:function tagOf(s: object): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %s: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS1.s]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS1.s]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = PrLoadInst (:string) %5: object, 0: number, "tag": string
// CHECK-NEXT:       ReturnInst %6: string
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [p: any]

// CHECK:function valueOf(p: object): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %p: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS2.p]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS2.p]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = PrLoadInst (:string|number) %5: object, 0: number, "k": string
// CHECK-NEXT:       ReturnInst %6: string|number
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [p: any]

// CHECK:function slotOf(p: object): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %p: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS3.p]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS3.p]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = LoadPropertyInst (:string) %5: object, "tag": string
// CHECK-NEXT:       ReturnInst %6: string
// CHECK-NEXT:function_end

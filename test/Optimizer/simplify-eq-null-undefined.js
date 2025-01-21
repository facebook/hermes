/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen %s

// Test equality comparisons against null|undefined when the other type can
// be neither type.

let o = {};
sink = o !== null;
sink = o === null;
sink = o != null;
sink = o == null;
sink = undefined !== o;
sink = undefined === o;
sink = undefined != o;
sink = undefined == o;

let n = +glob;
sink = n !== undefined;
sink = n === undefined;
sink = n != undefined;
sink = n == undefined;
sink = null !== n;
sink = null === n;
sink = null != n;
sink = null == n;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): boolean
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:       StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:       StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:       StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:       StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:       StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:       StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:       StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "glob": string
// CHECK-NEXT:  %9 = AsNumberInst (:number) %8: any
// CHECK-NEXT:        StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        StorePropertyLooseInst true: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        StorePropertyLooseInst false: boolean, globalObject: object, "sink": string
// CHECK-NEXT:        ReturnInst false: boolean
// CHECK-NEXT:function_end

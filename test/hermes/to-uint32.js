/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -O -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -exec %s | %FileCheck --match-full-lines %s

"use strict";

print('toUint32');
// CHECK-LABEL: toUint32

globalThis.x = -1;
print(globalThis.x >>> 0);
// CHECK-NEXT: 4294967295

globalThis.x = -1.5;
print(globalThis.x >>> 0);
// CHECK-NEXT: 4294967295

globalThis.x = 2247483648;
print(globalThis.x >>> 0);
// CHECK-NEXT: 2247483648

globalThis.x = 1;
print(globalThis.x >>> 0);
// CHECK-NEXT: 1

globalThis.x = 120938410947610748;
print(globalThis.x >>> 0);
// CHECK-NEXT: 272337024

globalThis.x = -120938410947610748;
print(globalThis.x >>> 0);
// CHECK-NEXT: 4022630272

globalThis.x = 17179867981.0;
print(globalThis.x >>> 0);
// CHECK-NEXT: 4294966093

/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('promise-disable');
// CHECK-LABEL: promise-disable

print(HermesInternal.hasPromise());
// CHECK-NEXT: false

print(typeof Promise);
// CHECK-NEXT: undefined

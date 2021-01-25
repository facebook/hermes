/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines --check-prefix=ON %s
// RUN: %hermes -Xes6-promise=0 %s | %FileCheck --match-full-lines --check-prefix=OFF %s

print('async depends on promise');;
// CHECK-LABEL: async depends on promise

async function empty() {};
try {
    print(empty())
} catch (e) {
    print(e);
}
// ON: [object Object]
// OFF: Error: async function cannot be used with Promise disabled. spawnAsync not registered.

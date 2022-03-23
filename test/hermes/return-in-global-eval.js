/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s

print(eval("10"));
//CHECK: 10

try {
    print(eval("return 10;"));
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught SyntaxError {{.*}}

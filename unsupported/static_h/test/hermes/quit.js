/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -O -target=HBC %s 2>&1 ) | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && (! %hermes %t.hbc 2>&1 ) | %FileCheck --match-full-lines %s

print("Start");
// CHECK-LABEL: Start
try {
    quit();
    print("Didn't quit");
} catch (e) {
    print("Caught:", e);
} finally {
    print("Finally");
}

// QuitError should be uncatchable, and should not run catch or finally blocks.

// CHECK-NEXT: Uncaught QuitError: Quit
// CHECK-NEXT:     at quit (native)

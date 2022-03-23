/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start < %s.step-in.debug %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-STEP-IN
// RUN: %hdb --break-at-start < %s.step-over.debug %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-STEP-OVER
// RUN: %hdb --break-at-start %s < %s.step.debug | %FileCheck --match-full-lines %s --check-prefix=CHECK-STEP
// RUN: %hdb --break-at-start %s < %s.finish.debug | %FileCheck --match-full-lines %s --check-prefix=CHECK-FINISH
// REQUIRES: debugger

var obj = {
    a: 'a',
    get aLen() {
        print("hello");
        return this.a.length;
    }
};

var alen = obj.aLen + 1;
print(alen);

// CHECK-STEP-OVER: Break on script load in global: {{.*}}:14:1
// CHECK-STEP-OVER: Stepped to global: {{.*}}:14:11
// CHECK-STEP-OVER: Stepped to global: {{.*}}:22:12
// CHECK-STEP-OVER: hello
// CHECK-STEP-OVER: Stepped to global: {{.*}}:23:1
// CHECK-STEP-OVER: 2

// CHECK-STEP-IN: Break on script load in global: {{.*}}:14:1
// CHECK-STEP-IN: Stepped to global: {{.*}}:14:11
// CHECK-STEP-IN: Stepped to global: {{.*}}:22:12
// CHECK-STEP-IN: Stepped to get aLen: {{.*}}:17:9
// CHECK-STEP-IN: hello
// CHECK-STEP-IN: Stepped to get aLen: {{.*}}:18:20

// CHECK-STEP: Break on script load in global: {{.*}}:14:1
// CHECK-STEP: Set breakpoint 1 at {{.*}}:17:9
// CHECK-STEP: Continuing execution
// CHECK-STEP: Break on breakpoint 1 in get aLen: {{.*}}:17:9
// CHECK-STEP: hello
// CHECK-STEP: Stepped to get aLen: {{.*}}:18:20
// CHECK-STEP: Stepped to global: {{.*}}:22:12
// CHECK-STEP: Stepped to global: {{.*}}:23:1
// CHECK-STEP: 2

// CHECK-FINISH: Break on script load in global: {{.*}}:14:1
// CHECK-FINISH: Set breakpoint 1 at {{.*}}:17:9
// CHECK-FINISH: Continuing execution
// CHECK-FINISH: Break on breakpoint 1 in get aLen: {{.*}}:17:9
// CHECK-FINISH: hello
// CHECK-FINISH: Stepped to global: {{.*}}:22:12
// CHECK-FINISH: Stepped to global: {{.*}}:23:1
// CHECK-FINISH: 2

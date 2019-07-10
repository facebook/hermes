// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hdb --break-at-start < %s.step-in.debug %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-STEP-IN
// RUN: %hdb --break-at-start < %s.step-over.debug %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-STEP-OVER
// REQUIRES: debugger

var obj = {};
obj[Symbol.toPrimitive] = function(hint) {
    print("hello");
    return 1;
};
if (obj == 1) {
    print("one");
}

// CHECK-STEP-IN: Break on script load in global: {{.*}}:10:1
// CHECK-STEP-IN: Stepped to global: {{.*}}:10:11
// CHECK-STEP-IN: Stepped to global: {{.*}}:11:1
// CHECK-STEP-IN: Stepped to global: {{.*}}:15:5
// CHECK-STEP-IN: Stepped to anonymous: {{.*}}:12:5
// CHECK-STEP-IN: hello
// CHECK-STEP-IN: Stepped to anonymous: {{.*}}:13:5

// CHECK-STEP-OVER: Break on script load in global: {{.*}}:10:1
// CHECK-STEP-OVER: Stepped to global: {{.*}}:10:11
// CHECK-STEP-OVER: Stepped to global: {{.*}}:11:1
// CHECK-STEP-OVER: Stepped to global: {{.*}}:15:5
// CHECK-STEP-OVER: hello
// CHECK-STEP-OVER: Stepped to global: {{.*}}:16:5
// CHECK-STEP-OVER: one

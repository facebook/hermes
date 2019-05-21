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

// CHECK-STEP-IN: Break on script load in global: {{.*}}:5:1
// CHECK-STEP-IN: Stepped to global: {{.*}}:5:11
// CHECK-STEP-IN: Stepped to global: {{.*}}:6:1
// CHECK-STEP-IN: Stepped to global: {{.*}}:10:5
// CHECK-STEP-IN: Stepped to anonymous: {{.*}}:7:5
// CHECK-STEP-IN: hello
// CHECK-STEP-IN: Stepped to anonymous: {{.*}}:8:5

// CHECK-STEP-OVER: Break on script load in global: {{.*}}:5:1
// CHECK-STEP-OVER: Stepped to global: {{.*}}:5:11
// CHECK-STEP-OVER: Stepped to global: {{.*}}:6:1
// CHECK-STEP-OVER: Stepped to global: {{.*}}:10:5
// CHECK-STEP-OVER: hello
// CHECK-STEP-OVER: Stepped to global: {{.*}}:11:5
// CHECK-STEP-OVER: one

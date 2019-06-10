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

// CHECK-STEP-OVER: Break on script load in global: {{.*}}:7:1
// CHECK-STEP-OVER: Stepped to global: {{.*}}:7:11
// CHECK-STEP-OVER: Stepped to global: {{.*}}:15:12
// CHECK-STEP-OVER: hello
// CHECK-STEP-OVER: Stepped to global: {{.*}}:16:1
// CHECK-STEP-OVER: 2

// CHECK-STEP-IN: Break on script load in global: {{.*}}:7:1
// CHECK-STEP-IN: Stepped to global: {{.*}}:7:11
// CHECK-STEP-IN: Stepped to global: {{.*}}:15:12
// CHECK-STEP-IN: Stepped to get aLen: {{.*}}:10:9
// CHECK-STEP-IN: hello
// CHECK-STEP-IN: Stepped to get aLen: {{.*}}:11:20

// CHECK-STEP: Break on script load in global: {{.*}}:7:1
// CHECK-STEP: Set breakpoint 1 at {{.*}}:10:9
// CHECK-STEP: Continuing execution
// CHECK-STEP: Break on breakpoint 1 in get aLen: {{.*}}:10:9
// CHECK-STEP: hello
// CHECK-STEP: Stepped to get aLen: {{.*}}:11:20
// CHECK-STEP: Stepped to global: {{.*}}:15:12
// CHECK-STEP: Stepped to global: {{.*}}:16:1
// CHECK-STEP: 2

// CHECK-FINISH: Break on script load in global: {{.*}}:7:1
// CHECK-FINISH: Set breakpoint 1 at {{.*}}:10:9
// CHECK-FINISH: Continuing execution
// CHECK-FINISH: Break on breakpoint 1 in get aLen: {{.*}}:10:9
// CHECK-FINISH: hello
// CHECK-FINISH: Stepped to global: {{.*}}:15:12
// CHECK-FINISH: Stepped to global: {{.*}}:16:1
// CHECK-FINISH: 2

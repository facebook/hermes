// RUN: %hdb --break-at-start < %s.step-in.debug %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-STEP-IN
// RUN: %hdb --break-at-start < %s.step-over.debug %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-STEP-OVER
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

// CHECK-STEP-OVER: Break on script load in global: {{.*}}:5:1
// CHECK-STEP-OVER: Stepped to global: {{.*}}:5:11
// CHECK-STEP-OVER: Stepped to global: {{.*}}:13:12
// CHECK-STEP-OVER: hello
// CHECK-STEP-OVER: Stepped to global: {{.*}}:14:1
// CHECK-STEP-OVER: 2

// CHECK-STEP-IN: Break on script load in global: {{.*}}:5:1
// CHECK-STEP-IN: Stepped to global: {{.*}}:5:11
// CHECK-STEP-IN: Stepped to global: {{.*}}:13:12
// CHECK-STEP-IN: Stepped to get aLen: {{.*}}:8:9
// CHECK-STEP-IN: hello
// CHECK-STEP-IN: Stepped to get aLen: {{.*}}:9:20

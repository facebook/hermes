// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;
for (var i = 0; i < 5; ++i) {
  print('iteration', i);
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:4:1
// CHECK-NEXT: Stepped to global: {{.*}}:5:12
// CHECK-NEXT: Stepped to global: {{.*}}:6:3
// CHECK-NEXT: iteration 0
// CHECK-NEXT: Stepped to global: {{.*}}:5:26
// CHECK-NEXT: Stepped to global: {{.*}}:6:3
// CHECK-NEXT: iteration 1
// CHECK-NEXT: Stepped to global: {{.*}}:5:26
// CHECK-NEXT: Stepped to global: {{.*}}:6:3
// CHECK-NEXT: iteration 2
// CHECK-NEXT: Stepped to global: {{.*}}:5:26
// CHECK-NEXT: Stepped to global: {{.*}}:6:3
// CHECK-NEXT: iteration 3
// CHECK-NEXT: Stepped to global: {{.*}}:5:26
// CHECK-NEXT: Stepped to global: {{.*}}:6:3
// CHECK-NEXT: iteration 4

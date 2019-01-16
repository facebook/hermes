// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('foo');
  print('bar');
print('baz');

// CHECK: Break on script load in global: {{.*}}:4:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:5:3
// CHECK-NEXT: 1 E {{.*}}break-line.js:5:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo
// CHECK-NEXT: Break on breakpoint 1 in global: {{.*}}:5:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: bar
// CHECK-NEXT: baz

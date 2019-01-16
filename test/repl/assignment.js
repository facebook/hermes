// RUN: cat %s | %repl -prompt "" 2>&1 | %FileCheck --match-full-lines %s

"assignment"
// CHECK-LABEL: "assignment"
x = 1
// CHECK-NEXT: 1
y = x + 1
// CHECK-NEXT: 2
y = x / 2
// CHECK-NEXT: 0.5

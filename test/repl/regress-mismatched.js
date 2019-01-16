// RUN: cat %s | %repl -prompt "" -prompt2 "" | %FileCheck --match-full-lines %s

new Set[1,2,3])
// CHECK: SyntaxError: 1:14:';' expected
// CHECK-NEXT: at evaluateLine ({{.*}})

// RUN: cat %s | %repl -prompt "" -prompt2 "" | %FileCheck --match-full-lines %s

throw new SyntaxError();
// CHECK: SyntaxError
// CHECK-NEXT: at eval {{.*}}
// CHECK-NEXT: at evaluateLine {{.*}}

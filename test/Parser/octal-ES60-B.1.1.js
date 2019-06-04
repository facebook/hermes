// RUN: ( %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s
09;
//CHECK: {{.*}}octal-ES60-B.1.1.js:2:1: warning: Numeric literal starts with 0 but contains an 8 or 9 digit. Interpreting as decimal (not octal).
//CHECK-NEXT: 09;
//CHECK-NEXT: ^~

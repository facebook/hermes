// RUN: ( ! %hermesc -dump-ir -ferror-limit=2 %s 2>&1 ) | %FileCheck --match-full-lines %s

1.a2
//CHECK: {{.*}}too-many-errors.js:3:1: error: invalid numeric literal
//CHECK-NEXT: 1.a2
//CHECK-NEXT: ^~~~

1.a3
//CHECK: {{.*}}too-many-errors.js:8:1: error: invalid numeric literal
//CHECK-NEXT: 1.a3
//CHECK-NEXT: ^~~~
//CHECK-NEXT: <unknown>:0: error: too many errors emitted

1.a4
1.a5

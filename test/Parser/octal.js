// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

"use strict";

"\001";  // this should be ok
x = 010;
//CHECK: {{.*}}octal.js:6:5: error: Octal literals are not allowed in strict mode
//CHECK-NEXT: x = 010;
//CHECK-NEXT:     ^~~

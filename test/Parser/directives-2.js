// RUN: (! %hermes -non-strict %s 2>&1 ) | %FileCheck --match-full-lines %s

// Make sure we scan directive prologues.

"use the force"
"use strict"
010
//CHECK: {{.*}}directives-2.js:7:1: error: Octal literals are not allowed in strict mode
//CHECK-NEXT: 010
//CHECK-NEXT: ^~~

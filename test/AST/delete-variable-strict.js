// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

"use strict";

delete a;
//CHECK: {{.*}}delete-variable-strict.js:5:1: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT: delete a;
//CHECK-NEXT: ^~~~~~~~

// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

function foo(p, p) {
//CHECK: {{.*}}param_redeclaration.js:3:17: error: cannot declare two parameters with the same name 'p'
//CHECK-NEXT: function foo(p, p) {
//CHECK-NEXT:                 ^

  "use strict";
  return p + p;
}

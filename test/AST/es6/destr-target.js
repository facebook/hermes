// RUN: (%hermes -hermes-parser %s 2>&1 || true) | %FileCheck %s --match-full-lines

({a : 0} = x)
//CHECK: {{.*}}destr-target.js:3:7: error: invalid assignment left-hand side
//CHECK-NEXT: ({a : 0} = x)
//CHECK-NEXT:       ^

({a : function a() {} } = x)
//CHECK: {{.*}}destr-target.js:8:7: error: invalid assignment left-hand side
//CHECK-NEXT: ({a : function a() {} } = x)
//CHECK-NEXT:       ^~~~~~~~~~~~~~~

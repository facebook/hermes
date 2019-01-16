// RUN: %hermes -Wno-direct-eval -O %s | %FileCheck --match-full-lines %s
"use strict";

print('strict eval');
// CHECK-LABEL: strict eval

var x = 3;
eval('x = 4');
print(x);
// CHECK-NEXT: 4

function f() {
  print('called f');
}

eval('f()');
// CHECK-NEXT: called f

eval('print(123)');
// CHECK-NEXT: 123

try {
    eval('"use strict"; y = 4');
} catch (e) {
    print(e.name, e.message);
}
// CHECK-NEXT: ReferenceError {{.*}}

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
"use strict";

try {
    x = 10;
} catch(e) {
    print(e.name, e.message);
}
// CHECK: ReferenceError {{.*}}

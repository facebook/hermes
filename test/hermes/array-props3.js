// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

var x = Array(5);
print(x);
//CHECK: ,,,,

Object.preventExtensions(x);

try {
    x[0] = 1;
} catch(e) {
    print(e.name, e.message);
}
//CHECK-NEXT: TypeError {{.*}}

try {
    Object.defineProperty(x, 0, {writable:true, configurable:true, enumerable:true, value:1});
} catch(e) {
    print(e.name, e.message);
}
//CHECK-NEXT: TypeError {{.*}}

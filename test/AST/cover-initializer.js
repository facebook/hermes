// RUN: (! %hermesc -dump-ir %s 2>&1 ) | %FileCheck --match-full-lines %s

var tmp = {a: 10, b = 30};
//CHECK: {{.*}}cover-initializer.js:3:21: error: ':' expected in property initialization
//CHECK-NEXT: var tmp = {a: 10, b = 30};
//CHECK-NEXT:                     ^

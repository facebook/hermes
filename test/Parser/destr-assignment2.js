// RUN: (! %hermesc -dump-ast --pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

({a : x = 10, get b() {}} = x)
//CHECK: {{.*}}destr-assignment2.js:3:15: error: invalid destructuring target
//CHECK-NEXT: ({a : x = 10, get b() {}} = x)
//CHECK-NEXT:               ^~~~~

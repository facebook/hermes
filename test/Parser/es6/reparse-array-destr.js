// RUN: (! %hermesc -dump-ir %s 2>&1 ) | %FileCheck --match-full-lines %s

[((a)), [(b)], c] = t;

([a, b]) = t;
//CHECK: {{.*}}reparse-array-destr.js:5:2: error: invalid assignment left-hand side
//CHECK-NEXT: ([a, b]) = t;
//CHECK-NEXT:  ^~~~~~

[(a = 1)] = t;
//CHECK: {{.*}}reparse-array-destr.js:10:3: error: invalid assignment left-hand side
//CHECK-NEXT: [(a = 1)] = t;
//CHECK-NEXT:   ^~~~~

[([b])] = t;
//CHECK: {{.*}}reparse-array-destr.js:15:3: error: invalid assignment left-hand side
//CHECK-NEXT: [([b])] = t;
//CHECK-NEXT:   ^~~

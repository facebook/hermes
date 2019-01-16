// RUN: cat %s | %repl -prompt "" | %FileCheck --match-full-lines %s

"wrapped eval"
// CHECK-LABEL: "wrapped eval"
{a:1}
// CHECK-NEXT: { a: 1 }
{if (true) {1} else {2}}
// CHECK-NEXT: 1
{if (false) {1} else {2}}
// CHECK-NEXT: 2

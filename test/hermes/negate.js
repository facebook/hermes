// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

var x = 5;
print(-x);
//CHECK: -5

x = -5.5;
print(-x);
//CHECK: 5.5

x = 4e30;
print(-x);
//CHECK: -4e+30

x = 0;
print(x);
print(-x);
print(1/x);
print(1/-x);
//CHECK: 0
//CHECK: 0
//CHECK: Infinity
//CHECK: -Infinity

x = "1";
print(-x);
//CHECK: -1


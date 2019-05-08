// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("BEGIN");
//CHECK: BEGIN

var {a, c, ...rest} = {x: 10, y: 20, c: 5, a: 3, z: 30}
print(a, c, Object.entries(rest));
//CHECK-NEXT: 3 5 x,10,y,20,z,30

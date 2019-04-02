// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

var C = {x: 2}
C[Symbol.hasInstance] = function(o) {
  // 'this' is C.
  // o is the object being tested for.
  print(this.x, o.isC);
  return o.isC;
}

print({isC:true} instanceof C);
// CHECK: 2 true
// CHECK: true

print({isC:false} instanceof C);
// CHECK: 2 false
// CHECK: false

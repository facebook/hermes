// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// This object is created from object buffer. Check it works.
var obj = {
  a : undefined,
  b : 1,
  c : {a : 3, b : 2},
  f : 1,
  g : 1,
  h : 1,
  i : 1,
  j : 1,
  k : 1,
  l : 1,
  m : 1,
  n : 1,
  o : function(x) {return x},
  p : 1,
  q : 1,
};

//CHECK: undefined
print(obj.a);
//CHECK-NEXT: 1
print(obj.f);
//CHECK-NEXT: 8
print(obj.o(8));
//CHECK-NEXT: 3
print(obj.c.a);
//CHECK-NEXT: 2
print(obj.c.b);

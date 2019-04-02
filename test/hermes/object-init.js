// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var obj = {a: 0, b: 1};
for (var p in obj) {
  print(p, obj[p]);
}
//CHECK: a 0
//CHECK: b 1


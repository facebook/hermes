// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

var obj = {a: 0, b: 1};
for (var p in obj) {
  print(p, obj[p]);
}
//CHECK: a 0
//CHECK: b 1


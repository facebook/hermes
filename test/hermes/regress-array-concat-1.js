// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var a = [];
a.__defineGetter__(0, function () {});
// Convert the result to string and print it
print(a.concat([])[0] + "foo");
//CHECK: undefinedfoo

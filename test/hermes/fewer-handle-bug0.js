// RUN: %hermes %s | %FileCheck --match-full-lines %s
var arr = [];
arr.length = 100;
function foo() { return "success"; }
var f = Function.prototype.bind.apply(foo, arr);
print(f());
//CHECK: success

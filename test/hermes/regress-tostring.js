// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Test that string conversion preserves the semantics of addition.

print("add-empty-string");
//CHECK-LABEL: add-empty-string

var o = Object('x');
o.valueOf = function() { return 'y' };
print(o + '');
//CHECK-NEXT: y
print('' + o);
//CHECK-NEXT: y

String.prototype.valueOf = function() { return 'y' };
var o = Object('x');
print(o + '');
//CHECK-NEXT: y
print('' + o);
//CHECK-NEXT: y

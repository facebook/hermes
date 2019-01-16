// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s
// Check that toPrimitive() is called with the correct hint

var x = {
    toString: function() { return "toString"; },
    valueOf: function() { return "valueOf"; }
};

print(String(x));
//CHECK: toString
print(x + "");
//CHECK-NEXT: valueOf

// RUN: %hermes -non-strict -target=HBC %s | %FileCheck --match-full-lines %s
// Make sure the global object has a prototype and prints as the correct class.

print(this);
//CHECK: [object global]

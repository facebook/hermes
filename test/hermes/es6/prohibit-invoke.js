// RUN: %hermes -Xflow-parser -O %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

var arrow1 = () => { print("in arrow"); }

try {
    new arrow1();
} catch (e) {
    print("caught", e.name, e.message);
//CHECK: caught TypeError Function is not a constructor
}

arrow1();
//CHECK-NEXT: in arrow

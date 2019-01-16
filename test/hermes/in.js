// RUN: %hermes -O -target=HBC %s | %FileCheck -match-full-lines %s

print("in");
//CHECK-LABEL: in

try {
    1 in 2;
} catch (e) {
    print("caught:", e);
}
//CHECK-NEXT: caught: TypeError: right operand of 'in' is not an object

print(1 in {});
//CHECK-NEXT: false
print(1 in [10,20]);
//CHECK-NEXT: true
print(1 in [10]);
//CHECK-NEXT: false
print(1 in {});
//CHECK-NEXT: false
print(1 in {1:10});
//CHECK-NEXT: true
print(1 in {2:20});
//CHECK-NEXT: false
print("a" in {});
//CHECK-NEXT: false
print("a" in {a:10});
//CHECK-NEXT: true

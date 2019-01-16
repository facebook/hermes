// RUN: %hermes -target=HBC -O -non-strict %s | %FileCheck --match-full-lines %s

x = 10;
print(x);
// CHECK: 10

try {
    print(y);
} catch (e) {
    print(e.name, e.message);
}
// CHECK-NEXT: ReferenceError {{.*}}

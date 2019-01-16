// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s

print(10);
//CHECK: 10
print(10.5);
//CHECK-NEXT: 10.5
print("hello");
//CHECK-NEXT: hello
print(void 0);
//CHECK-NEXT: undefined
print(null);
//CHECK-NEXT: null
print(true);
//CHECK-NEXT: true
print(false);
//CHECK-NEXT: false
print(0);
//CHECK-NEXT: 0

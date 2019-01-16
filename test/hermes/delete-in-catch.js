// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s

print("delete-in-catch");
//CHECK-LABEL: delete-in-catch

e = 100;
try {
    throw Error();
} catch (e) {
    print(delete e);
//CHECK-NEXT: false
}
print(e);
//CHECK-NEXT: 100

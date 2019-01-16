// RUN: %hermes -O -target=HBC -emit-binary -out=%t %s && %hermes -O -b %t | %FileCheck --match-full-lines %s
// Tests basic execution of bytecode (HBC).

print("bytecode");
//CHECK-LABEL: bytecode

e = 100;
try {
    throw Error();
} catch (e) {
    print(delete e);
//CHECK-NEXT: false
}
print(e);
//CHECK-NEXT: 100

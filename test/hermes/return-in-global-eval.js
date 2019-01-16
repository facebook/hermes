// RUN: %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s

print(eval("10"));
//CHECK: 10

try {
    print(eval("return 10;"));
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught SyntaxError {{.*}}

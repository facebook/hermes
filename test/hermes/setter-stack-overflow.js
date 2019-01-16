//RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s

// Ensure that deep recursion through a setter causes a regular stack overflow.

print("setter-stack-overflow");
//CHECK-LABEL: setter-stack-overflow

var obj = {set prop(v) { this.prop = v; } };
try {
    obj.prop = 10;
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught RangeError {{.*}}

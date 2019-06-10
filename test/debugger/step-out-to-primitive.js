// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

var obj = {};
obj[Symbol.toPrimitive] = function(hint) {
  if (hint === "number") {
    return 1;
  } else {
    return true;
  }
}

print("negate");
print(-obj);

print("add");
print("hello " + obj);

// CHECK: Break on script load in global: {{.*}}:4:1
// CHECK: Set breakpoint 1 at {{.*}}:6:7
// CHECK: Continuing execution
// CHECK: negate
// CHECK: Break on breakpoint 1 in anonymous: {{.*}}:6:7
// CHECK: Stepped to global: {{.*}}:14:7
// CHECK: Continuing execution
// CHECK: -1
// CHECK: add
// CHECK: Break on breakpoint 1 in anonymous: {{.*}}:6:7
// CHECK: Stepped to global: {{.*}}:17:7
// CHECK: Continuing execution
// CHECK: hello true

// RUN: %hermes -Xflow-parser %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

function hello(thing: string): string {
  return "hello " + thing;
}

// CHECK-LABEL: hello world
print(hello("world"));

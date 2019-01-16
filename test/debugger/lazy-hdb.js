// RUN: %hdb --lazy %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('foo called');
}

function bar() {
  print('bar called');
}

print('hello');
// CHECK: hello
foo();
// CHECK: foo called

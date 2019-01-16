// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('foo');
// CHECK: foo
